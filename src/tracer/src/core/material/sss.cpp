#include <agz/tracer/core/material.h>

#include "./bssrdf/normalized_diffusion.h"

AGZ_TRACER_BEGIN

class SubsurfaceScatteringWrapper : public Material
{
    RC<const Texture2D> A_;
    RC<const Texture2D> dmfp_;
    RC<const Texture2D> eta_;
    RC<const Material> internal_;

public:

    explicit SubsurfaceScatteringWrapper(
        RC<const Texture2D> A, RC<const Texture2D> dmfp, RC<const Texture2D> eta,
        RC<const Material> internal) noexcept
        : A_(std::move(A)), dmfp_(std::move(dmfp)), eta_(std::move(eta)),
          internal_(std::move(internal))
    {
        
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        ShadingPoint shd = internal_->shade(inct, arena);
        if(!shd.bssrdf)
        {
            const Spectrum A    = A_->sample_spectrum(inct.uv);
            const Spectrum dmfp = dmfp_->sample_spectrum(inct.uv);
            const real eta      = eta_->sample_real(inct.uv);

            const BSSRDF *bssrdf = arena.create<NormalizedDiffusionBSSRDF>(
                inct, eta, A, dmfp);

            shd.bssrdf = bssrdf;
        }

        return shd;
    }
};

RC<Material> create_sss_wrapper(
    RC<const Texture2D> A,
    RC<const Texture2D> dmfp,
    RC<const Texture2D> eta,
    RC<const Material> internal)
{
    return newRC<SubsurfaceScatteringWrapper>(
        std::move(A), std::move(dmfp), std::move(eta), std::move(internal));
}

AGZ_TRACER_END
