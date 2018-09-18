#pragma once

#include "../Geometry.h"
#include "../Transform.h"

AGZ_NS_BEG(Atrc)

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

    bool HasIntersection(const Ray &ray) const override;

    Option<Intersection> EvalIntersection(const Ray &ray) const override;

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

AGZ_NS_END(Atrc)
