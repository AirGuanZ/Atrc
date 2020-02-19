#pragma once

#include <optional>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

namespace refl_aux
{
    
/**
 * @param w incident direction. from reflection point to outside
 * @param n reflection normal
 * 
 * both w and n should be normalized
 */
inline Vec3 reflect(const Vec3 &w, const Vec3 &n) noexcept
{
    return 2 * dot(w, n) * n - w;
}

/**
 * @param w incident direction. from refraction point to outside
 * @param n normal direction on the same side with w
 * @param eta incident IOR / refracted IOR
 * 
 * both w and n should be normalized
 */
inline std::optional<Vec3> refract(const Vec3 &w, const Vec3 &n, real eta) noexcept
{
    const real cos_theta_i = dot(w, n);
    const real sin2_theta_t = math::sqr(eta) * (1 - math::sqr(cos_theta_i));
    const real cos2_theta_t = 1 - sin2_theta_t;
    if(cos2_theta_t <= 0)
        return std::nullopt;
    const real cos_theta_t = std::sqrt(cos2_theta_t);
    return ((eta * cos_theta_i - cos_theta_t) * n - eta * w).normalize();
}

/**
 * @brief compute fresnel value of dielectric surface
 * 
 * @param eta_i inner IOR
 * @param eta_o outer IOR
 */
inline real dielectric_fresnel(real eta_i, real eta_o, real cos_theta_i) noexcept
{
    if(cos_theta_i < 0)
    {
        std::swap(eta_i, eta_o);
        cos_theta_i = -cos_theta_i;
    }

    const real sin_theta_i = std::sqrt((std::max)(real(0), 1 - cos_theta_i * cos_theta_i));
    const real sin_theta_t = eta_o / eta_i * sin_theta_i;

    if(sin_theta_t >= 1)
        return 1;

    const real cos_theta_t = std::sqrt((std::max)(real(0), 1 - sin_theta_t * sin_theta_t));
    const real para = (eta_i * cos_theta_i - eta_o * cos_theta_t) / (eta_i * cos_theta_i + eta_o * cos_theta_t);
    const real perp = (eta_o * cos_theta_i - eta_i * cos_theta_t) / (eta_o * cos_theta_i + eta_i * cos_theta_t);

    return real(0.5) * (para * para + perp * perp);
}

} // namespace refl_aux

AGZ_TRACER_END
