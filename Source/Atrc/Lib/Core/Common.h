#pragma once

#include <cstdint>

#include <AGZUtils/Utils/Alloc.h>
#include <AGZUtils/Utils/Math.h>

namespace AGZ
{
    template<typename Pixel>
    class Texture2D;
}

namespace Atrc
{
// ================================= Memory

using Arena = AGZ::ObjArena<>;

// ================================= Real Number

using Real = float;
using RealT = AGZ::Math::FP<Real>;

extern const Real EPS;

// ================================= Math

using Vec2  = AGZ::Math::Vec2<Real>;
using Vec3  = AGZ::Math::Vec3<Real>;
using Vec4  = AGZ::Math::Vec4<Real>;
using Vec2u = AGZ::Math::Vec2<uint32_t>;
using Vec2i = AGZ::Math::Vec2<int32_t>;

using Rect  = AGZ::Math::Rect<Real>;
using Rectu = AGZ::Math::Rect<uint32_t>;
using Recti = AGZ::Math::Rect<int32_t>;

using Mat3 = AGZ::Math::RM_Mat3<Real>;
using Mat4 = AGZ::Math::RM_Mat4<Real>;

using Rad = AGZ::Math::Rad<Real>;
using Deg = AGZ::Math::Deg<Real>;

using AGZ::Math::Abs;
using AGZ::Math::ApproxEq;
using AGZ::Math::Arccos;
using AGZ::Math::Arcsin;
using AGZ::Math::Arctan2;
using AGZ::Math::Clamp;
using AGZ::Math::Cos;
using AGZ::Math::Exp;
using AGZ::Math::Log_e;
using AGZ::Math::Min;
using AGZ::Math::Max;
using AGZ::Math::Pow;
using AGZ::Math::Saturate;
using AGZ::Math::Sin;
using AGZ::Math::Sqrt;
using AGZ::Math::Tan;

constexpr Real PI = AGZ::Math::PI<Real>;
constexpr Real InvPI = AGZ::Math::InvPI<Real>;

inline Real Cos(const Vec3 &L, const Vec3 &R) noexcept
{
    return Dot(L, R) / (L.Length() * R.Length());
}

// ================================= (theta, phi) in local coord

// w must be normalized
inline Real CosTheta(const Vec3 &w) noexcept
{
    return w.z;
}

// w must be normalized
inline Real TanTheta(const Vec3 &w)
{
    Real t = 1 - w.z * w.z;
    if(t <= 0)
        return 0;
    return Sqrt(t) / w.z;
}

// w must be normalized
inline Real TanTheta2(const Vec3 &w)
{
    Real z2 = w.z * w.z;
    Real t = 1 - z2;
    if(t <= 0)
        return 0;
    return t / z2;
}

inline Real Phi(const Vec3 &w) noexcept
{
    if(!w.y && !w.x)
        return 0;
    Real ret = Arctan2(w.y, w.x);
    return ret < 0 ? (ret + 2 * PI) : ret;
}

// ================================= Spectrum

using AGZ::Math::Color3b;
using AGZ::Math::Color3f;

using Spectrum = AGZ::Math::Color3<Real>;

constexpr int SPECTRUM_CHANNEL_COUNT = 3;

namespace SPECTRUM = AGZ::Math::COLOR;

inline bool operator!(const Spectrum &s) noexcept
{
    return s == SPECTRUM::BLACK;
}

// ================================= Image

using Image = AGZ::Texture2D<Spectrum>;

// ================================= Important Interface

class Camera;
class Entity;
class FilmFilter;
class Geometry;
class Light;
class Material;
class Medium;
class PhaseFunction;
class PostProcessor;
class Ray;
class Renderer;
class Reporter;
class Sampler;
class Scene;
class Texture;

struct Intersection;

template<typename> class TFilm;
using Film = TFilm<Spectrum>;

template<typename> class TFilmGrid;
using FilmGrid = TFilmGrid<Spectrum>;

// ================================= Exception

class Exception
{
    std::string msg_;

public:

    explicit Exception(std::string msg) noexcept : msg_(std::move(msg)) { }

    const std::string &What() const noexcept { return msg_; }
};

#define DEFINE_ATRC_EXCEPTION(NAME) \
    class NAME : public ::Atrc::Exception \
    { \
    public: \
        explicit NAME(std::string err) : Exception(std::move(err)) noexcept { } \
    }

} // namespace Atrc
