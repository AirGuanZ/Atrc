#include <atomic>
#include <iostream>

#include <Utils/Thread.h>
#include <Utils/Time.h>

#include <Atrc/Lib/Core/TFilm.h>
#include <Atrc/SH2D/SceneProjector.h>

namespace Atrc::SH2D
{

SceneProjector::SceneProjector(
    int workerCount, int taskGridSize, int SHOrder,
    int minDepth, int maxDepth, Real contProb) noexcept
    : workerCount_(workerCount), taskGridSize_(taskGridSize), SHC_(SHOrder * SHOrder),
      minDepth_(minDepth), maxDepth_(maxDepth), contProb_(contProb)
{
    AGZ_ASSERT(taskGridSize > 0);
    AGZ_ASSERT(0 < SHOrder && SHOrder <= 4);
    AGZ_ASSERT(1 <= minDepth && minDepth <= maxDepth);
    AGZ_ASSERT(0 < contProb && contProb <= 1);
}

void SceneProjector::Project(const Scene &scene, Sampler *sampler, ProjectResult *result) const
{
    AGZ_ASSERT(sampler && result);

    auto resolution = result->albedoMap->GetResolution();
    std::queue<Grid> tasks = GridDivider<int32_t>::Divide(
        { { 0, 0 }, { resolution.x, resolution.y } }, taskGridSize_, taskGridSize_);

    size_t totalTaskCount = tasks.size();
    std::atomic<size_t> finishedTaskCount = 0;

    std::mutex mergeMut;

    auto func = [&](const Grid &task, AGZ::NoSharedParam_t)
    {
        int32_t taskID = task.low.x * int32_t(tasks.size()) + task.low.y;
        auto gridSampler = sampler->Clone(taskID);
        
        std::vector<FilmGrid> coefsGrid;
        for(int i = 0; i < SHC_; ++i)
            coefsGrid.push_back(result->coefs[i].CreateFilmGrid(task));
        TFilmGrid<Real> binaryMaskGrid = result->binaryMask->CreateFilmGrid(task);
        FilmGrid albedoMapGrid         = result->albedoMap->CreateFilmGrid(task);
        FilmGrid normalMapGrid         = result->normalMap->CreateFilmGrid(task);

        ProjectResultGrid resultGrid = {
            coefsGrid.data(),
            &binaryMaskGrid,
            &albedoMapGrid,
            &normalMapGrid
        };

        ProjectGrid(scene, gridSampler.get(), &resultGrid);

        for(int i = 0; i < SHC_; ++i)
            result->coefs[i].MergeFilmGrid(coefsGrid[i]);
        result->binaryMask->MergeFilmGrid(binaryMaskGrid);
        result->albedoMap->MergeFilmGrid(albedoMapGrid);
        result->normalMap->MergeFilmGrid(normalMapGrid);

        std::lock_guard<std::mutex> lk(mergeMut);

        Real percent = Real(100) * ++finishedTaskCount / totalTaskCount;
        std::cout << "Progress: " << percent << std::endl;
    };

    AGZ::Clock clock;

    AGZ::StaticTaskDispatcher<Grid, AGZ::NoSharedParam_t> dispatcher(workerCount_);
    std::cout << "Start rendering..." << std::endl;
    clock.Restart();

    bool ok = dispatcher.Run(func, AGZ::NO_SHARED_PARAM, tasks);

    if(!ok)
    {
        std::cout << "Something was wrong...";
        for(auto &err : dispatcher.GetExceptions())
            std::cout << err.what();
    }
    else
        std::cout << "Complete rendering..."
                  << "Total time: " << clock.Milliseconds() / 1000.0 << std::endl;
}

void SceneProjector::ProjectGrid(
    const Scene &scene, Sampler *sampler, ProjectResultGrid *result) const
{
    Arena arena;

    auto cam = scene.GetCamera();
    auto rect = result->albedoMap->GetSamplingRect();

    std::vector<Spectrum> coefsPixel(SHC_);

    for(int32_t py = rect.low.y; py < rect.high.y; ++py)
    {
        for(int32_t px = rect.low.x; px < rect.low.x; ++px)
        {
            sampler->StartPixel({ px, py });
            do {
                auto camSam = sampler->GetCameraSample();
                auto [r, w, pdf] = cam->GenerateRay(camSam);

                ProjectResultPixel pixel;
                pixel.coefs = coefsPixel.data();

                Eval(scene, r, sampler, arena, &pixel);
                auto fac = w / pdf;

                for(int i = 0; i < SHC_; ++i)
                    result->coefs[i].AddSample(camSam.film, fac * pixel.coefs[i]);
                result->binaryMask->AddSample(camSam.film, fac.r * pixel.binary);
                result->albedoMap->AddSample(camSam.film, fac * pixel.albedo);
                result->normalMap->AddSample(camSam.film, fac * pixel.normal);

                if(arena.GetUsedBytes() > 16 * 1024 * 1024)
                    arena.Clear();

            } while(sampler->NextSample());
        }
    }
}

namespace
{
    void ClearPixel(int SHOrder, SceneProjector::ProjectResultPixel *pixel)
    {
        for(int i = 0; i < SHOrder; ++i)
            pixel->coefs[i] = Spectrum();
        pixel->binary = 0;
        pixel->albedo = Spectrum();
        pixel->normal = Spectrum();
    }
}

void SceneProjector::Eval(
    const Scene &scene, const Ray &_r,
    Sampler *sampler, Arena &arena, ProjectResultPixel *output) const
{
    Spectrum coef = Spectrum(Real(1));
    ClearPixel(SHC_, output);
    Ray r = _r;

    auto SHTable = AGZ::Math::GetSHTable<Real>();

    for(int depth = 1; depth <= maxDepth_; ++depth)
    {
        // Russian roulette strategy

        if(depth > minDepth_)
        {
            if(sampler->GetReal() > contProb_)
                break;
            coef /= contProb_;
        }

        Intersection inct;
        if(!scene.FindIntersection(r, &inct))
        {
            if(depth > 1)
            {
                for(int i = 0; i < SHC_; ++i)
                    output->coefs[i] = coef * Spectrum(SHTable[i](r.d.Normalize()));
            }
            break;
        }

        ShadingPoint shd = inct.entity->GetMaterial(inct)->GetShadingPoint(inct, arena);

        if(depth == 1)
        {
            output->binary = 1;
            output->albedo = shd.bsdf->GetAlbedo(BSDF_ALL);
            output->normal = shd.coordSys.ez.Map([](Real c){ return (c + 1) / 2; });
        }

        auto bsdfSample = shd.bsdf->SampleWi(
            shd.coordSys, inct.coordSys, inct.wr, BSDF_ALL, sampler->GetReal2());
        if(!bsdfSample)
            break;
        
        r = Ray(inct.pos, bsdfSample->wi, EPS);
        coef *= bsdfSample->coef * Abs(Cos(shd.coordSys.ez, bsdfSample->wi)) / bsdfSample->pdf;
    }
}

} // namespace Atrc::SH2D
