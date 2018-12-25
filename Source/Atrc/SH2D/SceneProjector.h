#pragma once

#include <Utils/Texture.h>

#include <Atrc/Lib/Core/Common.h>
#include <Atrc/Lib/Core/Sampler.h>
#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Utility/GridDivider.h>

namespace Atrc::SH2D
{

class SceneProjector
{
public:

    struct ProjectResult
    {
        Film        *coefs;
        TFilm<Real> *binaryMask;
        Film        *albedoMap;
        Film        *normalMap;
    };

    struct ProjectResultPixel
    {
        Spectrum *coefs;
        int binary;
        Spectrum albedo;
        Spectrum normal;
    };

    SceneProjector(
        int workerCount, int taskGridSize, int SHOrder,
        int minDepth, int maxDepth, Real contProb) noexcept;

    void Project(const Scene &scene, Sampler *sampler, ProjectResult *result) const;

private:

    int workerCount_;
    int taskGridSize_;

    int SHC_;

    int minDepth_, maxDepth_;
    Real contProb_;

    using Grid = GridDivider<int32_t>::Grid;

    struct ProjectResultGrid
    {
        FilmGrid        *coefs;
        TFilmGrid<Real> *binaryMask;
        FilmGrid        *albedoMap;
        FilmGrid        *normalMap;
    };

    void ProjectGrid(const Scene &scene, Sampler *sampler, ProjectResultGrid *result) const;

    void Eval(
        const Scene &scene, const Ray &_r,
        Sampler *sampler, Arena &arena, ProjectResultPixel *output) const;
};

} // namespace Atrc::SH2D
