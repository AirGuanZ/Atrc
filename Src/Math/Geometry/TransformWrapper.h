#pragma once

#include "../Geometry.h"

AGZ_NS_BEG(Atrc)

class TransformWrapper : public GeometryObject
{
    Transform local2World_;
    const GeometryObject *obj_;

protected:

    bool HasIntersectionImpl(
        const Ray &ray, Real minT, Real maxT
    ) const final override;

    Option<Intersection> EvalIntersectionImpl(
        const Ray &ray, Real minT, Real maxT
    ) const final override;

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

AGZ_NS_END(Atrc)
