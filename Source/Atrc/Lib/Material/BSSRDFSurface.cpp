#include <Atrc/Lib/Material/BSSRDF/NormalizedDiffusionBSSRDF.h>
#include <Atrc/Lib/Material/BSSRDFSurface.h>

namespace Atrc
{

BSSRDFSurface::BSSRDFSurface(
    const Material *surface, const Texture *AMap, const Texture *mfpMap, Real eta) noexcept
    : surface_(surface), AMap_(AMap), mfpMap_(mfpMap), eta_(eta)
{

}

ShadingPoint BSSRDFSurface::GetShadingPoint(const Intersection &inct, Arena &arena) const noexcept
{
    auto shd   = surface_->GetShadingPoint(inct, arena);
    
    auto A     = AMap_->Sample(shd.uv);
    auto mfp   = mfpMap_->Sample(shd.uv);
    shd.bssrdf = arena.Create<NormalizedDiffusionBSSRDF>(inct, eta_, A, mfp);

    return shd;
}

} // namespace Atrc
