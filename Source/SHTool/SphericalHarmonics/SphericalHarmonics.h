#pragma once

#include "../Common.h"

// See https://en.wikipedia.org/wiki/Table_of_spherical_harmonics
namespace SHImpl
{
    template<int L, int M> struct SHAux { };

    template<> struct SHAux<0, 0>
    {
        static Real Eval([[maybe_unused]] const Vec3 &dir)
        {
            static const Real COEF = Real(0.5) * Sqrt(1 / PI);
            return COEF;
        }
    };

    template<> struct SHAux<1, -1>
    {
        static Real Eval(const Vec3 &dir)
        {
            AGZ_ASSERT(IsNormalized(dir));
            static const Real COEF = Sqrt(3 / (4 * PI));
            return COEF * dir.y;
        }
    };

    template<> struct SHAux<1, 0>
    {
        static Real Eval(const Vec3 &dir)
        {
            AGZ_ASSERT(IsNormalized(dir));
            static const Real COEF = Sqrt(3 / (4 * PI));
            return COEF * dir.z;
        }
    };

    template<> struct SHAux<1, 1>
    {
        static Real Eval(const Vec3 &dir)
        {
            AGZ_ASSERT(IsNormalized(dir));
            static const Real COEF = Sqrt(3 / (4 * PI));
            return COEF * dir.x;
        }
    };

    template<> struct SHAux<2, -2>
    {
        static Real Eval(const Vec3 &dir)
        {
            AGZ_ASSERT(IsNormalized(dir));
            static const Real COEF = Real(0.5) * Sqrt(15 / PI);
            return COEF * dir.x * dir.y;
        }
    };

    template<> struct SHAux<2, -1>
    {
        static Real Eval(const Vec3 &dir)
        {
            AGZ_ASSERT(IsNormalized(dir));
            static const Real COEF = Real(0.5) * Sqrt(15 / PI);
            return COEF * dir.y * dir.z;
        }
    };

    template<> struct SHAux<2, 0>
    {
        static Real Eval(const Vec3 &dir)
        {
            AGZ_ASSERT(IsNormalized(dir));
            static const Real COEF = Real(0.25) * Sqrt(5 / PI);
            return COEF * (-dir.x * dir.x - dir.y * dir.y + 2 * dir.z * dir.z);
        }
    };

    template<> struct SHAux<2, 1>
    {
        static Real Eval(const Vec3 &dir)
        {
            AGZ_ASSERT(IsNormalized(dir));
            static const Real COEF = Real(0.5) * Sqrt(15 / PI);
            return COEF * dir.z * dir.x;
        }
    };

    template<> struct SHAux<2, 2>
    {
        static Real Eval(const Vec3 &dir)
        {
            AGZ_ASSERT(IsNormalized(dir));
            static const Real COEF = Real(0.25) * Sqrt(15 / PI);
            return COEF * (dir.x * dir.x - dir.y * dir.y);
        }
    };
}

template<int L, int M>
Atrc::Real SH(const Vec3 &dir)
{
    return SHImpl::SHAux<L, M>::Eval(dir);
}
