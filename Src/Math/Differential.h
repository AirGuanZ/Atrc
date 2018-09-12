#pragma once

#include <type_traits>

#include "AGZMath.h"
#include "Ray.h"

AGZ_NS_BEG(Atrc)

// Ray with additional differential information
template<typename R>
class DifferentialRayTemplate : public R
{
public:

    bool hasDifferential;

    Vec3r rxOrigin;
    Vec3r ryOrigin;
    Vec3r rxDirection;
    Vec3r ryDirection;

    DifferentialRayTemplate()
        : hasDifferential(false)
    {

    }

    template<typename...Args,
        std::enable_if_t<(AGZ::TypeOpr::TypeListLength_v<Args...> >= 1 &&
                          AGZ::TypeOpr::True_v<decltype(R(std::declval<Args>()...))>),
                         int> = 0>
        DifferentialRayTemplate(
            const Vec3r &rxOrigin,
            const Vec3r &ryOrigin,
            const Vec3r &rxDirection,
            const Vec3r &ryDirection,
            Args&&...args)
        : R(std::forward<Args>(args)...),
          hasDifferential(true),
          rxOrigin(rxOrigin),
          ryOrigin(ryOrigin),
          rxDirection(rxDirection),
          ryDirection(ryDirection)
    {

    }

    template<typename...Args,
        std::enable_if_t<(AGZ::TypeOpr::TypeListLength_v<Args...> >= 1 &&
                          AGZ::TypeOpr::True_v<decltype(R(std::declval<Args>()...))>),
                         int> = 0>
        explicit DifferentialRayTemplate(Args&&...args)
        : R(std::forward<Args>(args)...), hasDifferential(false)
    {

    }

    void NormalizeRX()
    {
        rxDirection = Normalize(rxDirection);
    }

    void NormalizeRY()
    {
        ryDirection = Normalize(ryDirection);
    }

    void NormalizeRXY()
    {
        NormalizeRX();
        NormalizeRY();
    }
};

using DifferentialRay  = DifferentialRayTemplate<Ray>;
using DifferentialRayT = DifferentialRayTemplate<RayT>;
using DifferentialRayR = DifferentialRayTemplate<RayR>;

// Local geometry information of a parameterized surface point
class SurfaceLocal
{
public:

    Vec3r position;
    Vec3r normal;
    Vec2r uv;
    Vec3r dpdu;
    Vec3r dpdv;

    SurfaceLocal() = default;

    SurfaceLocal(
        const Vec3r &pos,
        const Vec2r &uv,
        const Vec3r &nor,
        const Vec3r &dpdu, const Vec3r &dpdv)
        : position(pos),
          normal(nor),
          uv(uv),
          dpdu(dpdu),
          dpdv(dpdv)
    {

    }
};

AGZ_NS_END(Atrc)
