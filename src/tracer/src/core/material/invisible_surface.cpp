#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/material.h>

AGZ_TRACER_BEGIN

namespace
{
    class InvisibleSurfaceBSDF : public BSDF
    {
        FVec3 geometry_normal_;

    public:

        explicit InvisibleSurfaceBSDF(const FVec3 &geometry_normal)
            : geometry_normal_(geometry_normal)
        {
            
        }

        FSpectrum eval(const FVec3 &wi, const FVec3 &wo, TransMode mode) const override
        {
            return {};
        }

        BSDFSampleResult sample(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override
        {
            const real cosv = std::abs(cos(geometry_normal_, wo));
            const FSpectrum f = FSpectrum(1) / (cosv < EPS() ? 1 : cosv);
            return BSDFSampleResult(-wo, f, 1, true);
        }

        BSDFBidirSampleResult sample_bidir(const FVec3 &wo, TransMode mode, const Sample3 &sam) const override
        {
            const real cosv = std::abs(cos(geometry_normal_, wo));
            const FSpectrum f = FSpectrum(1) / (cosv < EPS() ? 1 : cosv);
            return BSDFBidirSampleResult(-wo, f, 1, 1, true);
        }

        real pdf(const FVec3 &wi, const FVec3 &wo) const override
        {
            return 0;
        }

        FSpectrum albedo() const override
        {
            return FSpectrum(1);
        }

        bool is_delta() const override
        {
            return true;
        }

        bool has_diffuse_component() const override
        {
            return false;
        }
    };
}

class InvisibleSurfaceMaterial final : public Material
{
    RC<const BSSRDFSurface> bssrdf_;

public:

    explicit InvisibleSurfaceMaterial(RC<const BSSRDFSurface> bssrdf) noexcept
        : bssrdf_(std::move(bssrdf))
    {
        
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        ShadingPoint shd;
        shd.bsdf = arena.create<InvisibleSurfaceBSDF>(
                                inct.geometry_coord.z);
        shd.shading_normal = inct.user_coord.z;
        shd.bssrdf = bssrdf_->create(inct, arena);
        return shd;
    }
};

RC<Material> create_invisible_surface(RC<const BSSRDFSurface> bssrdf)
{
    return newRC<InvisibleSurfaceMaterial>(std::move(bssrdf));
}

AGZ_TRACER_END
