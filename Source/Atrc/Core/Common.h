#pragma once

#include <memory>
#include <optional>

#include <Utils.h>

AGZ_NS_BEG(Atrc)

// ============================= Option =============================

using AGZ::Either;

using AGZ::Option;
using AGZ::Some;
using AGZ::None;

// ============================= RefCounted Object =============================

template<typename T>
using RC = std::shared_ptr<T>;

template<typename T, typename...Args>
auto MakeRC(Args&&...args) { return std::make_shared<T>(std::forward<Args>(args)...); }

template<typename T>
using Box = std::unique_ptr<T>;

template<typename T, typename...Args>
auto MakeBox(Args&&...args) { return std::make_unique<T>(std::forward<Args>(args)...); }

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

DEFINE_ATRC_EXCEPTION(ArgumentException);
DEFINE_ATRC_EXCEPTION(UnimplementedMethod);
DEFINE_ATRC_EXCEPTION(UnreachableException);

// ============================= Atrc::Math =============================

using Real = double;
using RealT = AGZ::Math::FP<Real>;

constexpr Real EPS = Real(1e-6);

using Vec2 = AGZ::Math::Vec2<Real>;
using Vec3 = AGZ::Math::Vec3<Real>;
using Vec4 = AGZ::Math::Vec4<Real>;

using Mat3 = AGZ::Math::Mat3<Real>;
using Mat4 = AGZ::Math::Mat4<Real>;

using Rad = AGZ::Math::Rad<Real>;
using Deg = AGZ::Math::Deg<Real>;

using AGZ::Math::Color3f;
using AGZ::Math::Color3b;

inline Real Rand() { return AGZ::Math::Random::Uniform(Real(0), Real(1)); }

inline Real Cos(const Vec3 &L, const Vec3 &R) { return Dot(L, R) / (L.Length() * R.Length()); }

inline bool IsNormalized(const Vec3 &v) { return AGZ::Math::ApproxEq(v.LengthSquare(), Real(1), EPS); }

using AGZ::Math::Abs;
using AGZ::Math::ApproxEq;
using AGZ::Math::Clamp;
using AGZ::Math::Exp;
using AGZ::Math::Log_e;
using AGZ::Math::Max;
using AGZ::Math::Min;
using AGZ::Math::Pow;
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

// ============================= Atrc::String =============================

using AGZ::Str8;
using AGZ::StrView8;

// ============================= 一些针对BSDF空间的辅助函数 =============================

inline Real CosTheta(const Vec3 &w) { return w.z; }
inline Real Cos2Theta(const Vec3 &w) { return w.z * w.z; }
inline Real Sin2Theta(const Vec3 &w) { return Max(Real(0), 1 - Cos2Theta(w)); }
inline Real SinTheta(const Vec3 &w) { return Sqrt(Sin2Theta(w)); }
inline Real CosPhi(const Vec3 &w) { Real s = SinTheta(w); return !s ? Real(1) : Clamp(w.x / s, Real(-1), Real(1)); }
inline Real SinPhi(const Vec3 &w) { Real s = SinTheta(w); return !s ? Real(0) : Clamp(w.y / s, Real(-1), Real(1)); }

// ============================= Spectrum =============================

using Spectrum = Color3f;

namespace SPECTRUM = AGZ::Math::COLOR;

constexpr int SPECTRUM_CHANNEL_COUNT = 3;

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
class Medium;
class PhaseFunction;
class Ray;
class Renderer;
class Scene;
class Transform;

struct MediumPoint;
struct MediumShadingPoint;

struct SurfacePoint;
struct ShadingPoint;

using RenderTarget = AGZ::Texture2D<Spectrum>;

AGZ_NS_END(Atrc)
