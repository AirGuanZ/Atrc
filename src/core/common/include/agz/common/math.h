#pragma once

#include <limits>

#include <agz/utility/math.h>

namespace agz
{

using real = float;

constexpr real PI_r = math::PI<real>;
constexpr real invPI_r = 1 / PI_r;

constexpr real REAL_INF = std::numeric_limits<real>::infinity();
constexpr real REAL_MAX = std::numeric_limits<real>::max();
constexpr real REAL_MIN = std::numeric_limits<real>::lowest();

using Vec2 = math::tvec2<real>;
using Vec3 = math::tvec3<real>;
using Vec4 = math::tvec4<real>;

using Vec2i = math::vec2i;
using Vec3i = math::vec3i;

using Mat3 = math::tmat3_c<real>;
using Mat4 = math::tmat4_c<real>;

using Trans4 = Mat4::left_transform;

using Coord      = math::tcoord3<real>;
using Transform2 = math::ttransform2<real>;
using Transform3 = math::ttransform3<real>;

using Spectrum = math::tcolor3<real>;

constexpr int SPECTRUM_COMPONENT_COUNT = 3;

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

} // namespace agz
