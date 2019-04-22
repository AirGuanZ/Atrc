#include <queue>

#include <AGZUtils/Utils/Console.h>
#include <AGZUtils/Utils/Thread.h>
#include <AGZUtils/Utils/Time.h>
#include <Atrc/Core/Core/Camera.h>
#include <Atrc/Core/Core/TFilm.h>
#include <Atrc/Core/Core/Sampler.h>
#include <Atrc/Core/Core/Scene.h>
#include <Atrc/Core/Utility/GridDivider.h>
#include <Atrc/SH2D/Scene2SH.h>

using namespace Atrc;

namespace
{
    struct Scene2SHGrid
    {
        FilmGrid *coefs  = nullptr;
        FilmGrid *binary = nullptr;
        FilmGrid *albedo = nullptr;
        FilmGrid *normal = nullptr;
    };

    struct Scene2SHPixel
    {
        Spectrum *coefs = nullptr;
        Spectrum binary;
        Spectrum albedo;
        Spectrum normal;
    };

    void ClearPixelCoefs(Scene2SHPixel *pixel, int SHC)
    {
        for(int i = 0; i < SHC; ++i)
            pixel->coefs[i] = Spectrum();
    }

    void ProcessPixel(const Ray &_ray, int SHC, const Scene &scene, Sampler *sampler, Scene2SHPixel *pixel, Arena &arena)
    {
        Ray ray = _ray.Normalize();
        auto SHTable = AGZ::Math::SH::GetSHTable<Real>();

        Intersection inct;
        if(!scene.FindIntersection(ray, &inct))
        {
            for(int i = 0; i < SHC; ++i)
                pixel->coefs[i] = Spectrum(SHTable[i](ray.d));
            pixel->binary = Spectrum();
            pixel->albedo = Spectrum();
            pixel->normal = Spectrum();
            return;
        }

        ShadingPoint shd = inct.material->GetShadingPoint(inct, arena);

        pixel->binary = Spectrum(1);
        pixel->albedo = shd.bsdf->GetBaseColor(BSDF_ALL);
        pixel->normal = shd.coordSys.ez.Map([](Real c) { return (c + 1) / 2; });

        auto bsdfSample = shd.bsdf->SampleWi(
            shd.coordSys, inct.coordSys, inct.wr,
            BSDF_ALL, false, sampler->GetReal3());
        if(!bsdfSample)
        {
            ClearPixelCoefs(pixel, SHC);
            return;
        }

        Spectrum uncoloredCoef = shd.bsdf->EvalUncolored(
            shd.coordSys, inct.coordSys, bsdfSample->wi, inct.wr,
            BSDF_ALL, false) * Abs(Cos(shd.coordSys.ez, bsdfSample->wi)) / bsdfSample->pdf;

        ray = Ray(inct.pos + EPS * inct.coordSys.ez, bsdfSample->wi, 0).Normalize();
        if(scene.HasIntersection(ray))
        {
            ClearPixelCoefs(pixel, SHC);
            return;
        }

        for(int i = 0; i < SHC; ++i)
            pixel->coefs[i] = uncoloredCoef * SHTable[i](ray.d);
    }

    void ProcessGrid(int SHC, const Scene &scene, Sampler *sampler, Scene2SHGrid *grid)
    {
        Arena arena;

        const Camera *camera = scene.GetCamera();
        Recti rect = grid->albedo->GetSamplingRect();
        std::vector<Spectrum> coefsInPixel(SHC);

        for(int32_t py = rect.low.y; py < rect.high.y; ++py)
        {
            for(int32_t px = rect.low.x; px < rect.high.x; ++px)
            {
                sampler->StartPixel({ px, py });
                do {
                    auto cameraSample = sampler->GetCameraSample();
                    auto [ray, weight, pdf] = camera->GenerateRay(cameraSample);

                    Scene2SHPixel pixel;
                    pixel.coefs = coefsInPixel.data();
                    ProcessPixel(ray, SHC, scene, sampler, &pixel, arena);

                    Spectrum factor = weight / pdf;

                    for(int i = 0; i < SHC; ++i)
                        grid->coefs[i].AddSample(cameraSample.film, factor * pixel.coefs[i]);

                    grid->binary->AddSample(cameraSample.film, factor * pixel.binary);
                    grid->albedo->AddSample(cameraSample.film, factor * pixel.albedo);
                    grid->normal->AddSample(cameraSample.film, factor * pixel.normal);

                    if(arena.GetUsedBytes() > 32 * 1024 * 1024)
                        arena.Clear();

                } while(sampler->NextSample());
            }
        }
    }
}

void Scene2SH(
    int workerCount, int taskGridSize, int SHOrder,
    const Scene &scene, Sampler *sampler, Scene2SHResult *result)
{
    AGZ_ASSERT(sampler && result);

    using Grid = GridDivider<int32_t>::Grid;
    int SHC = (SHOrder + 1) * (SHOrder + 1);

    Vec2i resolution = result->albedo->GetResolution();

    std::vector<Grid> tasks0;
    std::queue<Grid> tasks;
    
    GridDivider<int32_t>::Divide(
        { { 0, 0 }, resolution }, taskGridSize, taskGridSize, std::back_inserter(tasks0));
    for(auto &grid : tasks0)
        tasks.push(grid);
    tasks0.clear();
    tasks0.shrink_to_fit();
    
    size_t totalTaskCount = tasks.size();
    std::atomic<size_t> finishedTaskCount = 0;

    std::mutex mergeMut;

    AGZ::StaticTaskDispatcher<Grid, AGZ::NoSharedParam_t> dispatcher(workerCount);
    
    AGZ::ProgressBarF pbar(80, '=');

    auto func = [&](const Grid &task, AGZ::NoSharedParam_t)
    {
        int32_t taskID = task.low.x * int32_t(tasks.size()) + task.low.y;
        auto gridSampler = sampler->Clone(taskID);

        std::vector<FilmGrid> coefsGrid;
        for(int i = 0; i < SHC; ++i)
            coefsGrid.push_back(result->coefs[i].CreateFilmGrid(task));
        FilmGrid binaryGrid = result->binary->CreateFilmGrid(task);
        FilmGrid albedoGrid = result->albedo->CreateFilmGrid(task);
        FilmGrid normalGrid = result->normal->CreateFilmGrid(task);

        Scene2SHGrid resultGrid = {
            coefsGrid.data(),
            &binaryGrid,
            &albedoGrid,
            &normalGrid
        };
        ProcessGrid(SHC, scene, gridSampler.get(), &resultGrid);

        for(int i = 0; i < SHC; ++i)
            result->coefs[i].MergeFilmGrid(coefsGrid[i]);
        result->binary->MergeFilmGrid(binaryGrid);
        result->albedo->MergeFilmGrid(albedoGrid);
        result->normal->MergeFilmGrid(normalGrid);

        std::lock_guard<std::mutex> lk(mergeMut);

        Real percent = Real(100) * ++finishedTaskCount / totalTaskCount;
        pbar.SetPercent(percent);
        pbar.Display();
    };

    bool ok = dispatcher.Run(func, AGZ::NO_SHARED_PARAM, tasks);

    if(!ok)
    {
        std::cout << "Something was wrong..." << std::endl;
        for(auto &err : dispatcher.GetExceptions())
            std::cout << err.what() << std::endl;
    }
    pbar.Done();
}
