#pragma once

#include <Atrc/Core/Common.h>
#include <Atrc/Core/SurfacePoint.h>

AGZ_NS_BEG(Atrc)

enum BxDFType
{
    BXDF_NONE     = 0,

    BXDF_DIFFUSE  = 1 << 0,
    BXDF_GLOSSY   = 1 << 1,
    BXDF_SPECULAR = 1 << 2,

    BXDF_REFLECTION   = 1 << 3,
    BXDF_TRANSMISSION = 1 << 4,

    BXDF_ALL = BXDF_DIFFUSE | BXDF_GLOSSY | BXDF_SPECULAR |
               BXDF_REFLECTION | BXDF_TRANSMISSION
};

template<typename T>
bool Match(T myType, BxDFType dstType)
{
    return (BxDFType(myType) & dstType) == myType;
}

struct BSDFSampleWiResult
{
    Vec3 wi;
    Real pdf = 0.0;
    Spectrum coef;
    BxDFType type = BXDF_NONE;
};

class BSDF
{
protected:

    LocalCoordSystem shadingLocal_;
    LocalCoordSystem geometryLocal_;

public:

    BSDF(const Vec3 &shadingNormal, const LocalCoordSystem &geometryLocal)
        : shadingLocal_(LocalCoordSystem::FromEz(shadingNormal)), geometryLocal_(geometryLocal)
    {
        shadingLocal_.ex = shadingLocal_.ex.Normalize();
        shadingLocal_.ey = shadingLocal_.ey.Normalize();
        shadingLocal_.ez = shadingLocal_.ez.Normalize();
    }

    virtual ~BSDF() = default;

    virtual Spectrum Eval(const Vec3 &wi, const Vec3 &wo, BxDFType type) const = 0;

    // 给定wo，对wi进行type类型的采样
    virtual Option<BSDFSampleWiResult> SampleWi(const Vec3 &wo, BxDFType type) const = 0;

    // 给定某wi，假设这时SampleWi的采样结果，返回pdf
    virtual Real SampleWiPDF(const Vec3 &wi, const Vec3 &wo, BxDFType type) const = 0;

    const LocalCoordSystem &GetShadingLocal() const { return shadingLocal_; }
    const LocalCoordSystem &GetGeometryLocal() const { return geometryLocal_; }
};

AGZ_NS_END(Atrc)
