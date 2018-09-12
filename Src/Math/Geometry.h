#pragma once

#include <optional>
#include <Utils/Math.h>

#include "Differential.h"
#include "Ray.h"
#include "Transform.h"

AGZ_NS_BEG(Atrc)

struct Intersection
{
    Real t;
    SurfaceLocal inct;
};

class GeometryObject
{
protected:

    virtual bool HasIntersectionImpl(
        const Ray &ray, Real minT, Real maxT) const
    {
        return EvalIntersectionImpl(ray, minT, maxT).has_value();
    }

    virtual std::optional<Intersection> EvalIntersectionImpl(
        const Ray &ray, Real minT, Real maxT) const = 0;

public:

    virtual ~GeometryObject() = default;

    bool HasIntersection(
        const Ray &ray,
        Real minT = Real(0.0),
        Real maxT = FP<Real>::Max()) const
    {
        return HasIntersectionImpl(ray, minT, maxT);
    }

    std::optional<Intersection> EvalIntersection(
        const Ray &ray,
        Real minT = Real(0.0),
        Real maxT = FP<Real>::Max()) const
    {
        return EvalIntersectionImpl(ray, minT, maxT);
    }
};

class TransformWrapper : public GeometryObject
{
    Transform local2World_;
    const GeometryObject *obj_;

protected:

    bool HasIntersectionImpl(
        const Ray &ray, Real minT, Real maxT
    ) const final override
    {
        return obj_->HasIntersection(
            Ray(
                local2World_.ApplyInverseToPoint(ray.origin),
                local2World_.ApplyInverseToVector(ray.direction)),
            minT, maxT);
    }

    Option<Intersection> EvalIntersectionImpl(
        const Ray &ray, Real minT, Real maxT
    ) const final override
    {
        auto tret = obj_->EvalIntersection(
            Ray(
                local2World_.ApplyInverseToPoint(ray.origin),
                local2World_.ApplyInverseToVector(ray.direction)),
            minT, maxT);

        if(!tret)
            return std::nullopt;
        return Intersection{
            tret->t,
            local2World_.ApplyToSurfaceLocal(tret->inct)
        };
    }

public:

    TransformWrapper(
        const Transform &local2World,
        const GeometryObject *wrappedObj)
        : local2World_(local2World), obj_(wrappedObj)
    {
        AGZ_ASSERT(wrappedObj);
    }

    const Transform &Local2World() const
    {
        return local2World_;
    }

    void SetLocal2World(
        const Transform &local2World = Transform::StaticIdentity())
    {
        local2World_ = local2World;
    }

    const GeometryObject *GetGeometryObject() const
    {
        return obj_;
    }
};

class GeometryObjectWithTransform : public GeometryObject
{
protected:

    const Transform *local2World_;

public:

    explicit GeometryObjectWithTransform(
        const Transform *local2World = &Transform::StaticIdentity())
        : local2World_(local2World)
    {
        AGZ_ASSERT(local2World_);
    }

    const Transform *GetTransform() const
    {
        return local2World_;
    }

    void SetTransform(const Transform *local2World)
    {
        AGZ_ASSERT(local2World);
        local2World_ = local2World;
    }
};

AGZ_NS_END(Atrc)
