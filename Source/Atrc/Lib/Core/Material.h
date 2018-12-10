#pragma once

#include <Atrc/Lib/Core/Common.h>
#include <Atrc/Lib/Core/SurfacePoint.h>

namespace Atrc
{

enum BSDFComponentType
{
    BSDF_DIFFUSE  = (1 << 0),
    BSDF_GLOSSY   = (1 << 1),
    BSDF_SPECULAR = (1 << 2),

    BSDF_REFLECTION   = (1 << 3),
    BSDF_TRANSMISSION = (1 << 4),

    BSDF_NONE = 0,
    BSDF_ALL  = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION
};

bool Contains(BSDFComponentType set, BSDFComponentType subset) noexcept;

class BSDF
{
public:

    struct SampleWiResult
    {
        Vec3 wi;
        Real pdf;
        Spectrum coef;
        BSDFComponentType type;
    };

    virtual ~BSDF() = default;

    virtual Spectrum Eval(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo) const noexcept = 0;

    virtual Option<SampleWiResult> SampleWi(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wo) const noexcept = 0;

    virtual Real SampleWiPDF(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo) const noexcept = 0;
};

class Material
{
public:

    virtual ~Material() = default;

    virtual ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const = 0;
};

} // namespace Atrc
