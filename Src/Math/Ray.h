#pragma once

#include <utility>
#include <Utils/Misc.h>

#include "AGZMath.h"

AGZ_NS_BEG(Atrc)

class Ray
{
public:

    Vec3r origin;
    Vec3r direction;

    Ray() = default;

    Ray(const Vec3r &ori, const Vec3r &dir)
        : origin(ori), direction(dir)
    {

    }

    Vec3r At(Real t) const
    {
        return origin + t * direction;
    }

    void Normalize()
    {
        direction = AGZ::Math::Normalize(direction);
    }
};

class RayT : public Ray
{
public:

    Real t;

    RayT(Real t = Real(0.0))
        : Ray(), t(t)
    {

    }

    RayT(const Vec3r &ori, const Vec3r &dir, Real t = Real(0.0))
        : Ray(ori, dir), t(t)
    {

    }

    Vec3r At(Real t) const
    {
        return Ray::At(t);
    }

    Vec3r At() const
    {
        return At(t);
    }

    bool IsNonNegative() const
    {
        return t >= Real(0.0);
    }
};

template<typename R>
class DifferentialRayTemplate : public T
{
public:

    bool hasDifferential;

    Vec3r rxOrigin;
    Vec3r ryOrigin;
    Vec3r rxDirection;
    Vec3r ryDirection;

    template<typename...Args,
             std::enable_if_t<AGZ::TypeOpr::True_v<
                    decltype(T(std::declval<Args>(args)...))>, int> = 0>
    DifferentialRayTemplate(Args&&...args)
        : T(std::forward<Args>(args)...), hasDifferential(false)
    {

    }
};

AGZ_NS_END(Atrc)
