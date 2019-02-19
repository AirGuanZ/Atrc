#pragma once

#include <Atrc/Lib/Material/Utility/BxDF.h>

namespace Atrc
{

// IMPROVE: anisotropic
// See https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
class DisneyPrincipledBxDF : public BxDF
{
    Spectrum rc_;
    Real roughness_;
    Real specular_;
    Real specularTint_;
    Real metallic_;
    Real sheen_;
    Real sheenTint_;
    Real subsurface_;
    Real clearCoat_;
    Real clearCoatGloss_;

public:

    DisneyPrincipledBxDF(
        const Spectrum &rc,
        Real roughness,
        Real specular,
        Real specularTint,
        Real metallic,
        Real sheen,
        Real sheenTint,
        Real subsurface,
        Real clearCoat,
        Real clearCoatGloss) noexcept
        : BxDF(BSDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
          rc_            (rc),
          roughness_     (roughness),
          specular_      (specular),
          specularTint_  (specularTint),
          metallic_      (metallic),
          sheen_         (sheen),
          sheenTint_     (sheenTint),
          subsurface_    (subsurface),
          clearCoat_     (clearCoat),
          clearCoatGloss_(clearCoatGloss)
    {

    }

    Spectrum Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override
    {
        if(wi.z <= 0 || wo.z <= 0 || !geoInShd.InPositiveHemisphere(wi))
            return Spectrum();

        // TODO
        return Spectrum();
    }

    std::optional<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override
    {
        // TODO
        return std::nullopt;
    }

    Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override
    {
        // TODO
        return 0;
    }
};

} // namespace Atrc
