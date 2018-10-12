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

        BxDFType GetType() const override;

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

        Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override;

        Spectrum EmittedRadiance(const Intersection &inct) const override;
    };

    AmbientBRDF::AmbientBRDF(const Spectrum &color)
        : color_(color)
    {

    }

    BxDFType AmbientBRDF::GetType() const
    {
        return BXDF_NONE;
    }

    Spectrum AmbientBRDF::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        return SPECTRUM::BLACK;
    }

    Option<BxDFSample> AmbientBRDF::Sample(const Vec3r &wi, BxDFType type) const
    {
        return None;
    }

    Spectrum AmbientBRDF::EmittedRadiance(const Intersection &inct) const
    {
        return color_;
    }
}

AmbientMaterial::AmbientMaterial(const Spectrum &color)
    : color_(color)
{
    
}

Box<BxDF> AmbientMaterial::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewBox<AmbientBRDF>(color_);
}

AGZ_NS_END(Atrc)
