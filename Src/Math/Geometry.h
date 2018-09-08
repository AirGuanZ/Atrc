#pragma once

#include <optional>
#include <Utils/Math.h>

#include "Differential.h"
#include "Ray.h"

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

    virtual ~GeometryObject() = default;

    virtual bool HasIntersection(const Ray &ray) const
    {
        return EvalIntersection(ray).has_value();
    }

    virtual std::optional<Intersection> EvalIntersection(const Ray &ray) const
    {
        AGZ::Unreachable();
    }
};

class Transform
{
    Mat4r trans_;
    Mat4r invTrans_;

public:

    Transform()
        : trans_(Mat4r::IDENTITY()), invTrans_(Mat4r::IDENTITY())
    {
        
    }

    explicit Transform(const Mat4r &trans)
        : trans_(trans), invTrans_(Inverse(trans))
    {

    }

    Transform(const Mat4r &trans, const Mat4r &invTrans)
        : trans_(trans), invTrans_(invTrans)
    {
        
    }

    explicit Transform(const Mat4r *trans, const Mat4r *invTrans = nullptr)
        : trans_(AGZ::UNINITIALIZED), invTrans_(AGZ::UNINITIALIZED)
    {
        if(trans)
        {
            trans_    = *trans;
            invTrans_ = invTrans ? *invTrans : Inverse(trans_);
        }
        else
        {
            AGZ_ASSERT(invTrans);
            invTrans_ = *invTrans;
            trans_    = Inverse(invTrans_);
        }
    }

    static const Transform &StaticIdentity()
    {
        static const Transform ret;
        return ret;
    }
    
    Transform operator*(const Transform &rhs) const
    {
        return Transform{ trans_ * rhs.trans_, rhs.invTrans_ * invTrans_ };
    }

    Transform Reverse() const
    {
        return Transform(&invTrans_, &trans_);
    }

    Vec3r ApplyToPoint(const Vec3r &p) const
    {
        return trans_.ApplyToPoint(p);
    }

    Vec3r ApplyToVector(const Vec3r &v) const
    {
        return trans_.ApplyToVector(v);
    }

    Vec3r ApplyToNormal(const Vec3r &n) const
    {
        return invTrans_.ApplyInverseToNormal(n);
    }

    Vec3r ApplyInverseToPoint(const Vec3r &p) const
    {
        return invTrans_.ApplyToPoint(p);
    }

    Vec3r ApplyInverseToVector(const Vec3r &v) const
    {
        return invTrans_.ApplyToVector(v);
    }

    Vec3r ApplyInverseToNormal(const Vec3r &n) const
    {
        return trans_.ApplyInverseToNormal(n);
    }

    SurfaceLocal ApplyToSurfaceLocal(const SurfaceLocal &sl) const
    {
        return SurfaceLocal(
            ApplyToPoint(sl.position), sl.uv,
            ApplyToNormal(sl.dpdu), ApplyToNormal(sl.dpdv),
            ApplyToNormal(sl.dndu), ApplyToNormal(sl.dndv));
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
