#pragma once

#include <Atrc/Core/Common.h>

#include <Atrc/Core/AABB.h>
#include <Atrc/Core/Ray.h>
#include <Atrc/Core/SurfacePoint.h>
#include <Atrc/Core/Transform.h>

AGZ_NS_BEG(Atrc)

struct GeometrySampleResult
{
    Vec3 pos;
    Vec3 nor;
    Real pdf = 0.0;
};

class Geometry
{
protected:

    Transform local2World_;

public:

    explicit Geometry(const Transform &local2World)
        : local2World_(local2World)
    {
        
    }

    virtual ~Geometry() = default;

    virtual bool HasIntersection(const Ray &r) const;

    // 填充：t，pos，wo，geoUV，usrUV，geoLocal（，flag0）
    virtual bool FindIntersection(const Ray &r, SurfacePoint *sp) const = 0;

    // 在World意义上的表面积
    virtual Real SurfaceArea() const = 0;

    // 局部bound
    virtual AABB LocalBound() const = 0;

    // 世界bound
    virtual AABB WorldBound() const;

    // 在表面上采样
    virtual GeometrySampleResult Sample() const = 0;

    // 给定采样结果，返回其概率密度
    virtual Real SamplePDF(const Vec3 &pos) const = 0;

    // 以dst为“目标点”在表面上进行采样
    virtual GeometrySampleResult Sample(const Vec3 &dst) const;

    // 上一个函数的pdf
    virtual Real SamplePDF(const Vec3 &pos, const Vec3 &dst) const;
};

AGZ_NS_END(Atrc)
