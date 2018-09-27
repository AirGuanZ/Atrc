#pragma once

#include <type_traits>
#include <Utils.h>

#include "../Core/Spectrum.h"
#include "../Math/Geometry.h"
#include "../Math/Sample.h"

AGZ_NS_BEG(Atrc)

enum class BxDFType : uint8_t
{
    Reflection   = (1 << 0),
    Transmission = (1 << 1),
    Diffuse      = (1 << 2),
    Glossy       = (1 << 3),
    Specular     = (1 << 4),
    Ambient      = (1 << 5),
};

inline bool HasBxDFType(BxDFType typeSet, BxDFType typeTested)
{
    using UT = std::underlying_type_t<BxDFType>;
    return (static_cast<UT>(typeSet) &
            static_cast<UT>(typeTested))
           != 0;
}

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

    virtual BxDFType GetType() const = 0;

    virtual Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const = 0;

    virtual Option<BxDFSample> Sample(const Vec3r &wo) const = 0;

    virtual Real PDF(const Vec3r &wi, const Vec3r &wo) const = 0;

    virtual Spectrum AmbientRadiance() const = 0;
};

ATRC_INTERFACE Material
{
public:

    virtual ~Material() = default;

    virtual BxDF *GetBxDF(const SurfaceLocal &sl) const = 0;
};

AGZ_NS_END(Atrc)
