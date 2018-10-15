#pragma once

#include <Atrc/Common.h>

AGZ_NS_BEG(Atrc)

/*struct BxDFType
{
    uint8_t value;

    constexpr explicit BxDFType(uint8_t value) : value(value) {  }

    constexpr bool Contains(BxDFType contained) const
    {
        return (value & contained.value) == contained.value;
    }
};

constexpr BxDFType operator|(BxDFType lhs, BxDFType rhs)
{
    return BxDFType(lhs.value | rhs.value);
}

constexpr BxDFType BXDF_NONE         = BxDFType(0);
constexpr BxDFType BXDF_REFLECTION   = BxDFType(1 << 0);
constexpr BxDFType BXDF_TRANSMISSION = BxDFType(1 << 1);
constexpr BxDFType BXDF_DIFFUSE      = BxDFType(1 << 2);
constexpr BxDFType BXDF_GLOSSY       = BxDFType(1 << 3);
constexpr BxDFType BXDF_SPECULAR     = BxDFType(1 << 4);
constexpr BxDFType BXDF_ALL_REF      = BXDF_REFLECTION   | BXDF_DIFFUSE | BXDF_GLOSSY | BXDF_SPECULAR;
constexpr BxDFType BXDF_ALL_TRANS    = BXDF_TRANSMISSION | BXDF_DIFFUSE | BXDF_GLOSSY | BXDF_SPECULAR;
constexpr BxDFType BXDF_ALL          = BXDF_ALL_REF | BXDF_ALL_TRANS;*/

struct BxDFSample
{
    Vec3r dir;
    Spectrum coef;
    Real pdf;
};

ATRC_INTERFACE BxDF
{
public:

    virtual ~BxDF() = default;

    virtual Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const = 0;

    virtual Option<BxDFSample> Sample(const Vec3r &wi) const = 0;

    virtual Spectrum AmbientRadiance(const Intersection &inct) const;

    virtual bool CanScatter() const;

    virtual bool IsSpecular() const;

    virtual const BxDF *GetLeafBxDF() const;
};

AGZ_NS_END(Atrc)
