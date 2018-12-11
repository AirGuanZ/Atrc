#pragma once

#include <Utils/Alloc.h>
#include <Utils/Math.h>
#include <Utils/String.h>

namespace Atrc
{
// ================================= Memory

using Arena = AGZ::ObjArena<>;

// ================================= Real Number

using Real = float;
using RealT = AGZ::Math::FP<Real>;

extern const Real EPS;

// ================================= Option

using AGZ::Option;
using AGZ::Some;
using AGZ::None;

// ================================= String

using AGZ::Str8;

// ================================= Math

using Vec2  = AGZ::Math::Vec2<Real>;
using Vec3  = AGZ::Math::Vec3<Real>;
using Vec4  = AGZ::Math::Vec4<Real>;
using Vec2u = AGZ::Math::Vec2<uint32_t>;
using Vec2i = AGZ::Math::Vec2<int32_t>;

using Rect  = AGZ::Math::Rect<Real>;
using Rectu = AGZ::Math::Rect<uint32_t>;
using Recti = AGZ::Math::Rect<int32_t>;

using Mat3 = AGZ::Math::Mat3<Real>;
using Mat4 = AGZ::Math::Mat4<Real>;

using Rad = AGZ::Math::Rad<Real>;
using Deg = AGZ::Math::Deg<Real>;

using AGZ::Math::Abs;
using AGZ::Math::ApproxEq;
using AGZ::Math::Arcsin;
using AGZ::Math::Arctan2;
using AGZ::Math::Clamp;
using AGZ::Math::Min;
using AGZ::Math::Max;
using AGZ::Math::Saturate;
using AGZ::Math::Sqrt;

constexpr Real PI = AGZ::Math::PI<Real>;
constexpr Real InvPI = AGZ::Math::InvPI<Real>;

inline Real Rand() noexcept
{
    return AGZ::Math::Random::Uniform<Real>(0, 1);
}

inline Real Cos(const Vec3 &L, const Vec3 &R) noexcept
{
    return Dot(L, R) / (L.Length() * R.Length());
}

// ================================= Spectrum

using AGZ::Math::Color3f;
using AGZ::Math::Color3b;

using Spectrum = Color3f;

namespace SPECTRUM = AGZ::Math::COLOR;

inline bool operator!(const Spectrum &s) noexcept
{
    return s == SPECTRUM::BLACK;
}

// ================================= Exception

class Exception
{
    Str8 msg_;

public:

    explicit Exception(Str8 msg) noexcept : msg_(std::move(msg)) { }

    const Str8 What() const noexcept { return msg_; }
};

#define DEFINE_ATRC_EXCEPTION(NAME) \
    class NAME : public ::Atrc::Exception \
    { \
    public: \
        explicit NAME(Str8 err) : Exception(std::move(err)) noexcept { } \
    }

} // namespace Atrc
