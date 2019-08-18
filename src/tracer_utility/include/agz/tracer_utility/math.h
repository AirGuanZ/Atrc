#pragma once

#include <limits>

#include <agz/tracer_utility/common.h>
#include <agz/utility/math.h>

AGZ_TRACER_BEGIN

using real = float;

constexpr real PI_r = math::PI<real>;
constexpr real invPI_r = 1 / PI_r;

constexpr real REAL_INF = std::numeric_limits<real>::infinity();
constexpr real REAL_MAX = std::numeric_limits<real>::max();
constexpr real REAL_MIN = std::numeric_limits<real>::lowest();

extern real EPS;

using Vec2 = math::tvec2<real>;
using Vec3 = math::tvec3<real>;
using Vec4 = math::tvec4<real>;

using Vec2i = math::vec2i;
using Vec3i = math::vec3i;

using Mat3 = math::tmat3_c<real>;
using Mat4 = math::tmat4_c<real>;

using Coord      = math::tcoord3<real>;
using Transform2 = math::ttransform2<real>;
using Transform3 = math::ttransform3<real>;

using Spectrum = math::tcolor3<real>;

constexpr int SPECTRUM_COMPONENT_COUNT = 3;

struct Sample1 { real u; };
struct Sample2 { real u, v; };
struct Sample3 { real u, v, w; };
struct Sample4 { real u, v, w, r; };
struct Sample5 { real u, v, w, r, s; };

template<int N>
struct SampleN { real u[N]; };

/**
 * @brief 用0-255下的rgb三分量构造0-1下的spectrum
 */
inline Spectrum rgb255(real r, real g, real b) noexcept
{
    constexpr real ratio = real(1) / 255;
    return ratio * Spectrum(r, g, b);
}

/**
 * @brief 某spectrum中是否含有inf成分
 */
inline bool has_inf(const Spectrum &s) noexcept
{
    return std::isinf(s.r) || std::isinf(s.g) || std::isinf(s.b);
}

namespace local_angle
{

    /**
     * @brief 取得某向量与+z方向的夹角的cos值
     *
     * w must be normalized
     */
    inline real cos_theta(const Vec3 &w) noexcept
    {
        return w.z;
    }

    /**
     * @brief 将某角度的cos值转为其sin正值
     */
    inline real cos_2_sin(real cos)
    {
        return std::sqrt((std::max<real>)(0, 1 - cos * cos));
    }

    /**
     * @brief 求某向量与+z方向夹角的tan值
     *
     * w must be normalized
     */
    inline real tan_theta(const Vec3 &w)
    {
        real t = 1 - w.z * w.z;
        if(t <= 0)
            return 0;
        return std::sqrt(t) / w.z;
    }

    /**
     * @brief 求某向量与+z方向夹角的tan值的平方
     *
     * w must be normalized
     */
    inline real tan_theta_2(const Vec3 &w)
    {
        real z2 = w.z * w.z;
        real t = 1 - z2;
        if(t <= 0)
            return 0;
        return t / z2;
    }

    /**
     * @brief 求某向量在xy平面上的极角，范围[0-2pi]
     */
    inline real phi(const Vec3 &w) noexcept
    {
        if(!w.y && !w.x)
            return 0;
        real ret = std::atan2(w.y, w.x);
        return ret < 0 ? (ret + 2 * PI_r) : ret;
    }

    /**
     * @brief 求某向量与+z方向的夹角
     *
     * w must be normalized
     */
    inline real theta(const Vec3 &w) noexcept
    {
        return std::acos(math::clamp<real>(cos_theta(w), -1, 1));
    }

} // namespace local_angle

class Medium;

/**
 * @brief 由参数方程 o + td (t in [t_min, t_max]) 定义的射线（段）
 */
class Ray
{
public:

    Vec3 o;     // 起点
    Vec3 d;     // 方向
    real t_min; // 参数最小值
    real t_max; // 参数最大值

    Ray();
    Ray(const Vec3 &o, const Vec3 &d, real t_min = 0, real t_max = REAL_INF) noexcept;
    explicit Ray(uninitialized_t)                                                                            noexcept;

    Vec3 at(real t) const noexcept;

    bool between(real t) const noexcept;
};

/**
 * @brief 三维轴对齐包围盒
 *
 * 由包围盒对角线上的两点构成，low为包围盒上各维度最小值，high为包围盒上各维度最大值。
 *
 * 若low的每个分量都小于high的对应分量，称该包围盒是有效的，否则为无效。
 *
 * 包围盒上的基本运算是求并，即求出包含两个包围盒的最小包围盒。求并的零元称为零包围盒，是一个无效包围盒。
 */
class AABB
{
public:

    Vec3 low;
    Vec3 high;

    /** @brief 默认初始化为零包围盒 */
    AABB()                                  noexcept;

    /** @brief 用指定的两个关键点构造包围盒 */
    AABB(const Vec3 &low, const Vec3 &high) noexcept;

    /** @brief 构造一个未初始化的包围盒 */
    explicit AABB(uninitialized_t)          noexcept;

    /** @brief 返回该包围盒是否有效 */
    bool valid() const noexcept;

    /**
     * @brief 返回该包围盒的体积
     *
     * 对无效包围盒，该方法返回0
     */
    real volume() const noexcept;

    /** @brief 并入另一个包围盒 */
    AABB &operator|=(const AABB &rhs) noexcept;

    /** @brief 并入另一个点 */
    AABB &operator|=(const Vec3 &p) noexcept;

    /** @brief 是否包含某个点 */
    bool contains(const Vec3 &pnt) const noexcept
    {
        return low.x <= pnt.x && pnt.x <= high.x &&
               low.y <= pnt.y && pnt.y <= high.y &&
               low.z <= pnt.z && pnt.z <= high.z;
    }

    /** @brief 是否同某条参数射线有交点 */
    bool intersect(const Vec3 &ori, const Vec3 &inv_dir, real t_min, real t_max) const noexcept
    {
        real nx = inv_dir[0] * (low[0] - ori[0]);
        real ny = inv_dir[1] * (low[1] - ori[1]);
        real nz = inv_dir[2] * (low[2] - ori[2]);

        real fx = inv_dir[0] * (high[0] - ori[0]);
        real fy = inv_dir[1] * (high[1] - ori[1]);
        real fz = inv_dir[2] * (high[2] - ori[2]);

        t_min = (std::max)(t_min, (std::min)(nx, fx));
        t_min = (std::max)(t_min, (std::min)(ny, fy));
        t_min = (std::max)(t_min, (std::min)(nz, fz));

        t_max = (std::min)(t_max, (std::max)(nx, fx));
        t_max = (std::min)(t_max, (std::max)(ny, fy));
        t_max = (std::min)(t_max, (std::max)(nz, fz));

        return t_min <= t_max;
    }
};

/** @beif 求两个包围盒的并 */
AABB operator|(const AABB &lhs, const AABB &rhs) noexcept;

inline Ray::Ray()
    : Ray(Vec3(), Vec3(1, 0, 0))
{
    
}

inline Ray::Ray(const Vec3 &o, const Vec3 &d, real t_min, real t_max) noexcept
    : o(o), d(d), t_min(t_min), t_max(t_max)
{

}

inline Ray::Ray(uninitialized_t) noexcept
    : o(UNINIT), d(UNINIT)
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

inline bool AABB::valid() const noexcept
{
    return low.x < high.x && low.y < high.y && low.z < high.z;
}

inline real AABB::volume() const noexcept
{
    return valid() ? (high - low).product() : 0;
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

static_assert(sizeof(AABB) == 6 * sizeof(real));

AGZ_TRACER_END
