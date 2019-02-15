#include <Atrc/Lib/Material/BxDF/BxDF_Microfacet.h>

namespace Atrc
{

BxDF_MicrofacetReflection::BxDF_MicrofacetReflection(const Spectrum &rc, const MicrofacetDistribution *md, const Fresnel *fresnel) noexcept
    : BxDF(BSDFType(BSDF_GLOSSY | BSDF_REFLECTION)), rc_(rc), md_(md), fresnel_(fresnel)
{
    
}

} // namespace Atrc
