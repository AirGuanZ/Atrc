#pragma once

#include "AGZMath.h"
#include "Differential.h"

AGZ_NS_BEG(Atrc)

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
            trans_ = *trans;
            invTrans_ = invTrans ? *invTrans : Inverse(trans_);
        }
        else
        {
            AGZ_ASSERT(invTrans);
            invTrans_ = *invTrans;
            trans_ = Inverse(invTrans_);
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

AGZ_NS_END(Atrc)
