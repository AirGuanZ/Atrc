#include <Atrc/Material/Ambient.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class AmbientBRDF
        : ATRC_IMPLEMENTS BxDF,
          ATRC_PROPERTY AGZ::Uncopiable
    {
        Spectrum color_;

    public:

        explicit AmbientBRDF(const Spectrum &color);

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

        Real PDF(const Vec3r &wo, const Vec3r &sample) const override;

        Option<BxDFSample> Sample(const Vec3r &wo) const override;

        Spectrum AmbientRadiance(const Intersection &inct) const override;

        bool CanScatter() const override;
    };

    AmbientBRDF::AmbientBRDF(const Spectrum &color)
        : color_(color)
    {

    }

    Spectrum AmbientBRDF::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        return SPECTRUM::BLACK;
    }

    Real AmbientBRDF::PDF(const Vec3r &wo, const Vec3r &sample) const
    {
        return 0.0;
    }

    Option<BxDFSample> AmbientBRDF::Sample(const Vec3r &wo) const
    {
        return None;
    }

    Spectrum AmbientBRDF::AmbientRadiance(const Intersection &inct) const
    {
        return color_;
    }

    bool AmbientBRDF::CanScatter() const
    {
        return false;
    }
}

AmbientMaterial::AmbientMaterial(const Spectrum &color)
    : color_(color)
{
    
}

RC<BxDF> AmbientMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewRC<AmbientBRDF>(color_);
}

AGZ_NS_END(Atrc)
