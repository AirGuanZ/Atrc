#pragma once

#include <Atrc/Core/Common.h>

#include <Atrc/Core/AABB.h>
#include <Atrc/Core/Ray.h>
#include <Atrc/Core/SurfacePoint.h>
#include <Atrc/Core/Transform.h>

AGZ_NS_BEG(Atrc)

struct GeometrySampleResult
{
    SurfacePoint sp;
    Real pdf = 0.0;
};

class Geometry
{
    Transform local2World_;

public:

    explicit Geometry(const Transform &local2World)
        : local2World_(local2World)
    {
        
    }

    virtual ~Geometry() = default;

    virtual bool HasIntersection(const Ray &r) const;

    virtual bool FindIntersection(const Ray &r, SurfacePoint *sp) const = 0;

    virtual Real SurfaceArea() const = 0;

    virtual AABB LocalBound() const = 0;

    virtual AABB GlobalBound() const;

    virtual GeometrySampleResult Sample() const = 0;

    virtual Real SamplePDF(const SurfacePoint &sp) const = 0;
};

AGZ_NS_END(Atrc)
