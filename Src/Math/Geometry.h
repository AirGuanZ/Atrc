#pragma once

#include <optional>
#include <Utils/Math.h>

#include "Differential.h"
#include "Ray.h"
#include "Transform.h"

AGZ_NS_BEG(Atrc)

struct Intersection
{
    float t;
    float epsilon;
    SurfaceLocal inct;
};

class GeometryObject
{
public:

    virtual ~GeometryObject() { }

    virtual bool HasIntersection(const Ray &ray) const
    {
        return EvalIntersection(ray).has_value();
    }

    virtual std::optional<Intersection> EvalIntersection(const Ray &ray) const
    {
        AGZ::Unreachable();
    }
};

class TransformWrapper : public GeometryObject
{
    Transform local2World_;
    const GeometryObject *obj_;

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

    void SetLocal2World(const Transform &local2World)
    {
        local2World_ = local2World;
    }

    const GeometryObject *GetGeometryObject() const
    {
        return obj_;
    }

    bool HasIntersection(const Ray &ray) const override
    {
        return obj_->HasIntersection(Ray(
            local2World_.ApplyInverseToPoint(ray.origin),
            local2World_.ApplyInverseToVector(ray.direction)));
    }

    Option<Intersection> EvalIntersection(const Ray &ray) const override
    {
        auto tret = obj_->EvalIntersection(Ray(
            local2World_.ApplyInverseToPoint(ray.origin),
            local2World_.ApplyInverseToVector(ray.direction)));
        if(!tret)
            return std::nullopt;
        return Intersection {
            tret->t, tret->epsilon,
            local2World_.ApplyToSurfaceLocal(tret->inct)
        };
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
