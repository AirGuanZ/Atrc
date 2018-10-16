#include <Atrc/Material/Normal.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class NormalBRDF
        : ATRC_IMPLEMENTS BxDF,
        ATRC_PROPERTY AGZ::Uncopiable
    {
    public:

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

        Real PDF(const Vec3r &wo, const Vec3r &sample) const override;

        Option<BxDFSample> Sample(const Vec3r &wo) const override;

        Spectrum AmbientRadiance(const Intersection &inct) const override;
    };

    Spectrum NormalBRDF::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        return SPECTRUM::BLACK;
    }

    Real NormalBRDF::PDF(const Vec3r &wo, const Vec3r &sample) const
    {
        return 0.0;
    }

    Option<BxDFSample> NormalBRDF::Sample(const Vec3r &wo) const
    {
        return None;
    }

    Spectrum NormalBRDF::AmbientRadiance(const Intersection &inct) const
    {
        return 0.5f * (inct.nor.Map([](Real r) { return static_cast<float>(r); }) + Spectrum(1.0f));
    }
}

RC<BxDF> NormalMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewRC<NormalBRDF>();
}

AGZ_NS_END(Atrc)
