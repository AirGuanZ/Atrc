#pragma once

#include <stdexcept>

#include <agz/utility/alloc.h>
#include <agz/utility/texture.h>

#define AGZ_TRACER_BEGIN namespace agz::tracer {
#define AGZ_TRACER_END   }

#define AGZ_ANONYMOUS_NAMESPACE_BEGIN namespace {
#define AGZ_ANONYMOUS_NAMESPACE_END   }

#define AGZ_TRACER_FACTORY_BEGIN namespace agz::tracer::factory {
#define AGZ_TRACER_FACTORY_END   }

AGZ_TRACER_BEGIN

class Aggregate;
class BSDF;
class Camera;
class Entity;
class EnvirLight;
class FilmFilter;
class Geometry;
class Material;
class Medium;
class PostProcessor;
class Renderer;
class ProgressReporter;
class Sampler;
class Scene;
class Texture2D;
class Texture3D;

using real = float;

extern real EPS;

constexpr real PI_r = math::PI<real>;
constexpr real invPI_r = 1 / PI_r;

constexpr real REAL_INF = std::numeric_limits<real>::infinity();
constexpr real REAL_MAX = std::numeric_limits<real>::max();
constexpr real REAL_MIN = std::numeric_limits<real>::lowest();

using Vec2 = math::tvec2<real>;
using Vec3 = math::tvec3<real>;
using Vec4 = math::tvec4<real>;

using Rect2  = math::taabb2<real>;
using Rect2i = math::aabb2i;

using Vec2i = math::vec2i;
using Vec3i = math::vec3i;

using Mat3 = math::tmat3_c<real>;
using Mat4 = math::tmat4_c<real>;

using Trans4 = Mat4::left_transform;

using Coord = math::tcoord3<real>;
using Transform2 = math::ttransform2<real>;
using Transform3 = math::ttransform3<real>;

using Spectrum = math::tcolor3<real>;

constexpr int SPECTRUM_COMPONENT_COUNT = 3;

template<typename T>
using Image2D = texture::texture2d_t<T>;

template<typename T>
using Image3D = texture::texture3d_t<T>;

/**
 * @brief is there any inf component in given spectrum value
 */
inline bool has_inf(const Spectrum &s) noexcept
{
    return std::isinf(s.r) || std::isinf(s.g) || std::isinf(s.b);
}

using Arena = alloc::obj_arena_t;

class ObjectConstructionException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

namespace local_angle
{

    /**
     * w must be normalized
     */
    inline real cos_theta(const Vec3 &w) noexcept
    {
        return w.z;
    }

    inline real abs_cos_theta(const Vec3 &w) noexcept
    {
        return std::abs(cos_theta(w));
    }

    inline real cos_2_sin(real cos)
    {
        return std::sqrt((std::max<real>)(0, 1 - cos * cos));
    }

    /**
     * w must be normalized
     */
    inline real tan_theta(const Vec3 &w)
    {
        const real t = 1 - w.z * w.z;
        if(t <= 0)
            return 0;
        return std::sqrt(t) / w.z;
    }

    /**
     * w must be normalized
     */
    inline real tan_theta_2(const Vec3 &w)
    {
        const real z2 = w.z * w.z;
        const real t = 1 - z2;
        if(t <= 0)
            return 0;
        return t / z2;
    }

    inline real phi(const Vec3 &w) noexcept
    {
        if(!w.y && !w.x)
            return 0;
        const real ret = std::atan2(w.y, w.x);
        return ret < 0 ? (ret + 2 * PI_r) : ret;
    }

    /**
     * w must be normalized
     */
    inline real theta(const Vec3 &w) noexcept
    {
        return std::acos(math::clamp<real>(cos_theta(w), -1, 1));
    }

    /**
     * @brief correction factor for shading normal
     */
    inline real normal_corr_factor(const Vec3 &geo, const Vec3 &shd, const Vec3 &wi) noexcept
    {
        const real dem = std::abs(cos(geo, wi));
        return  dem < EPS ? 1 : std::abs(cos(shd, wi) / dem);
    }

    inline real normal_corr_factor(const Coord &geo, const Coord &shd, const Vec3 &wi) noexcept
    {
        return normal_corr_factor(geo.z, shd.z, wi);
    }

} // namespace local_angle

class Ray
{
public:

    Vec3 o;
    Vec3 d;
    real t_min;
    real t_max;

    Ray();
    Ray(const Vec3 &o, const Vec3 &d, real t_min = 0, real t_max = REAL_INF) noexcept;

    Vec3 at(real t) const noexcept;

    bool between(real t) const noexcept;
};

class AABB
{
public:

    Vec3 low;
    Vec3 high;

    /** @brief defaultly initialized to an invalid aabb */
    AABB() noexcept;

    AABB(const Vec3 &low, const Vec3 &high) noexcept;

    explicit AABB(uninitialized_t) noexcept;

    AABB &operator|=(const AABB &rhs) noexcept;

    AABB &operator|=(const Vec3 &p) noexcept;

    bool contains(const Vec3 &pnt) const noexcept
    {
        return low.x <= pnt.x && pnt.x <= high.x &&
               low.y <= pnt.y && pnt.y <= high.y &&
               low.z <= pnt.z && pnt.z <= high.z;
    }

    bool intersect(const Vec3 &ori, const Vec3 &inv_dir, real t_min, real t_max) const noexcept
    {
        const real nx = inv_dir[0] * (low[0] - ori[0]);
        const real ny = inv_dir[1] * (low[1] - ori[1]);
        const real nz = inv_dir[2] * (low[2] - ori[2]);

        const real fx = inv_dir[0] * (high[0] - ori[0]);
        const real fy = inv_dir[1] * (high[1] - ori[1]);
        const real fz = inv_dir[2] * (high[2] - ori[2]);

        t_min = (std::max)(t_min, (std::min)(nx, fx));
        t_min = (std::max)(t_min, (std::min)(ny, fy));
        t_min = (std::max)(t_min, (std::min)(nz, fz));

        t_max = (std::min)(t_max, (std::max)(nx, fx));
        t_max = (std::min)(t_max, (std::max)(ny, fy));
        t_max = (std::min)(t_max, (std::max)(nz, fz));

        return t_min <= t_max;
    }
};

AABB operator|(const AABB &lhs, const AABB &rhs) noexcept;

inline Ray::Ray()
    : Ray(Vec3(), Vec3(1, 0, 0))
{
    
}

inline Ray::Ray(const Vec3 &o, const Vec3 &d, real t_min, real t_max) noexcept
    : o(o), d(d), t_min(t_min), t_max(t_max)
{

}

inline Vec3 Ray::at(real t) const noexcept
{
    return o + t * d;
}

inline bool Ray::between(real t) const noexcept
{
    return t_min <= t && t <= t_max;
}

inline AABB::AABB() noexcept
    : low(REAL_MAX), high(REAL_MIN)
{

}

inline AABB::AABB(const Vec3 &low, const Vec3 &high) noexcept
    : low(low), high(high)
{

}

inline AABB::AABB(uninitialized_t) noexcept
    : low(UNINIT), high(UNINIT)
{

}

inline AABB &AABB::operator|=(const AABB &rhs) noexcept
{
    low.x = (std::min)(low.x, rhs.low.x);
    low.y = (std::min)(low.y, rhs.low.y);
    low.z = (std::min)(low.z, rhs.low.z);
    high.x = (std::max)(high.x, rhs.high.x);
    high.y = (std::max)(high.y, rhs.high.y);
    high.z = (std::max)(high.z, rhs.high.z);
    return *this;
}

inline AABB operator|(const AABB &lhs, const AABB &rhs) noexcept
{
    return AABB(Vec3((std::min)(lhs.low.x, rhs.low.x),
        (std::min)(lhs.low.y, rhs.low.y),
        (std::min)(lhs.low.z, rhs.low.z)),
        Vec3((std::max)(lhs.high.x, rhs.high.x),
        (std::max)(lhs.high.y, rhs.high.y),
            (std::max)(lhs.high.z, rhs.high.z)));
}

inline AABB &AABB::operator|=(const Vec3 &p) noexcept
{
    low.x = std::min(low.x, p.x);
    low.y = std::min(low.y, p.y);
    low.z = std::min(low.z, p.z);

    high.x = std::max(high.x, p.x);
    high.y = std::max(high.y, p.y);
    high.z = std::max(high.z, p.z);

    return *this;
}

struct Sample1 { real u; };
struct Sample2 { real u, v; };
struct Sample3 { real u, v, w; };
struct Sample4 { real u, v, w, r; };
struct Sample5 { real u, v, w, r, s; };

template<int N>
struct SampleN { real u[N]; };

static_assert(sizeof(AABB) == 6 * sizeof(real));

AGZ_TRACER_END
