#pragma once

#include <Atrc/Lib/Core/Common.h>
#include <Atrc/Lib/Core/SurfacePoint.h>

namespace Atrc
{

enum BSDFType
{
    BSDF_NONSPECULAR  = (1 << 0),
    BSDF_SPECULAR      = (1 << 1),

    BSDF_REFLECTION   = (1 << 2),
    BSDF_TRANSMISSION = (1 << 3),

    BSDF_NONE = 0,
    BSDF_NULL = (1 << 4),
    BSDF_ALL  = BSDF_NONSPECULAR | BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION
};

bool Contains(BSDFType set, BSDFType subset) noexcept;

class BSDF
{
public:

    struct SampleWiResult
    {
        Vec3 wi;
        Real pdf;
        Spectrum coef;
        BSDFType type;
        bool isDelta;
    };

    virtual ~BSDF() = default;

    virtual Spectrum GetBaseColor(BSDFType type) const noexcept = 0;

    virtual Spectrum Eval(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept = 0;

    // 满足约束：ret.coef = Eval, ret.pdf = SampleWiPDF
    virtual std::optional<SampleWiResult> SampleWi(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wo, BSDFType type, bool star, const Vec3 &sample) const noexcept = 0;

    virtual Real SampleWiPDF(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept = 0;

    // 返回BaseColor == WHITE时Eval出的结果
    // 一般来说应满足约束：Eval = BaseColor * EvalUncolored
    // 但不是所有的材质都做得到，这种情况下不对该函数的结果做任何要求
    virtual Spectrum EvalUncolored(
        const CoordSystem &shd, const CoordSystem &geo,
        const Vec3 &wi, const Vec3 &wo, BSDFType type, bool star) const noexcept = 0;
};

class Material
{
public:

    virtual ~Material() = default;

    virtual ShadingPoint GetShadingPoint(const Intersection &inct, Arena &arena) const = 0;
};

// ================================= Implementation

inline bool Contains(BSDFType set, BSDFType subset) noexcept
{
    return (subset & set) == subset;
}

} // namespace Atrc
