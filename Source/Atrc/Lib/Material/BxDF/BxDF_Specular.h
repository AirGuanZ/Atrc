#include <Atrc/Lib/Material/Utility/BxDF.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

class BxDF_Specular : public BxDF
{
    Spectrum rc_;
    const Dielectric *fresnel_;

public:

    BxDF_Specular(const Spectrum &rc, const Dielectric *fresnel);

    Spectrum GetAlbedo() const noexcept override;

    Spectrum Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept override;

    Option<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, const Vec2 &sample) const noexcept override;

    Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo) const noexcept override;
};

} // namespace Atrc
