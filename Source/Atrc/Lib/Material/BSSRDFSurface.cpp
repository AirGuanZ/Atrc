#include <Atrc/Lib/Material/BSSRDF/NormalizedDiffusionBSSRDF.h>
#include <Atrc/Lib/Material/BSSRDFSurface.h>

namespace Atrc
{

BSSRDFSurface::BSSRDFSurface(
    const Material *surface, const Texture *AMap, const Texture *dmfpMap, Real eta) noexcept
    : surface_(surface), AMap_(AMap), dmfpMap_(dmfpMap), eta_(eta)
{

}

ShadingPoint BSSRDFSurface::GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept
{
    auto shd   = surface_->GetShadingPoint(inct, arena);

    if(inct.coordSys.InPositiveHemisphere(inct.wr))
    {
        auto A     = AMap_->Sample(shd.uv);
        auto dmfp  = dmfpMap_->Sample(shd.uv);
        shd.bssrdf = arena.Create<NormalizedDiffusionBSSRDF>(inct, eta_, A, dmfp);
    }

    return shd;
}

} // namespace Atrc
