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

        explicit InvisibleSurfaceBSDF(const FVec3 &geometry_normal) noexcept
            : geometry_normal_(geometry_normal)
        {
            
        }

        Spectrum eval(
            const FVec3 &wi, const FVec3 &wo,
            TransMode mode, uint8_t) const noexcept override
        {
            return {};
        }

        BSDFSampleResult sample(
            const FVec3 &wo, TransMode mode,
            const Sample3 &sam, uint8_t type) const noexcept override
        {
            if(!(type & BSDF_SPECULAR))
                return BSDF_SAMPLE_RESULT_INVALID;

            const real cosv = std::abs(cos(geometry_normal_, wo));

            const Spectrum f = Spectrum(1) / (cosv < EPS() ? 1 : cosv);

            return BSDFSampleResult(-wo, f, 1, true);
        }

        real pdf(const FVec3 &wi, const FVec3 &wo, uint8_t) const noexcept override
        {
            return 0;
        }

        Spectrum albedo() const noexcept override
        {
            return Spectrum(1);
        }

        bool is_delta() const noexcept override
        {
            return true;
        }

        bool has_diffuse_component() const noexcept override
        {
            return false;
        }
    };
}

class InvisibleSurfaceMaterial : public Material
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
