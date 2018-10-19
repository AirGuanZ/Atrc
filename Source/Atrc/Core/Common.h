#pragma once

#include <memory>
#include <optional>

#include <Utils.h>

AGZ_NS_BEG(Atrc)

// ============================= Option =============================

template<typename T>
using Option = std::optional<T>;

template<typename T>
auto Some(T &&v) { return std::make_optional<T>(std::forward<T>(v)); }

constexpr std::nullopt_t None = std::nullopt;

// ============================= RefCounted Object =============================

template<typename T>
using RC = std::shared_ptr<T>;

template<typename T, typename...Args>
auto MakeRC(Args&&...args) { return std::make_shared<T>(std::forward<Args>(args)...); }

// ============================= Atrc::Exception =============================

class Exception : public std::runtime_error
{
public:
    explicit Exception(const std::string &err) : runtime_error(err) { }
};

#define DEFINE_ATRC_EXCEPTION(NAME) \
    class NAME : public ::Atrc::Exception \
    { \
    public: \
        explicit NAME(const std::string &err) : Exception(err) { } \
    }

#undef DEFINE_ATRC_EXCEPTION

// ============================= Atrc::Math =============================

using Real = double;
using RealT = AGZ::Math::FP<Real>;

using Vec2 = AGZ::Math::Vec2<Real>;
using Vec3 = AGZ::Math::Vec3<Real>;
using Vec4 = AGZ::Math::Vec4<Real>;

using Mat3 = AGZ::Math::Mat3<Real>;
using Mat4 = AGZ::Math::Mat4<Real>;

using Rad = AGZ::Math::Rad<Real>;
using Deg = AGZ::Math::Deg<Real>;

inline Real Rand() { return AGZ::Math::Random::Uniform(0.0, 1.0); }

inline Real Cos(const Vec3 &L, const Vec3 &R) { return Dot(L, R) / (L.Length() * R.Length()); }

using AGZ::Math::Abs;
using AGZ::Math::ApproxEq;
using AGZ::Math::Clamp;
using AGZ::Math::Max;
using AGZ::Math::Min;
using AGZ::Math::Sqrt;

using AGZ::Math::Cross;
using AGZ::Math::Dot;

using AGZ::Math::Sin;
using AGZ::Math::Cos;
using AGZ::Math::Tan;

using AGZ::Math::Arcsin;
using AGZ::Math::Arccos;
using AGZ::Math::Arctan2;

constexpr Real PI = AGZ::Math::PI<Real>;
constexpr Real InvPI = AGZ::Math::InvPI<Real>;

using AGZ::UNINITIALIZED;

// ============================= Spectrum =============================

using Spectrum = AGZ::Math::Color3f;

namespace SPECTRUM = AGZ::Math::COLOR;

inline bool operator!(const Spectrum &s)
{
    return s == SPECTRUM::BLACK;
}

// ============================= Forward declerations =============================

class AABB;
class BSDF;
class Camera;
class Entity;
class Geometry;
class Light;
class Integrator;
class Material;
class Ray;
class Renderer;
class Scene;
class Transform;

struct SurfacePoint;
struct ShadingPoint;

using RenderTarget = AGZ::Tex::Texture2D<Spectrum>;

AGZ_NS_END(Atrc)
