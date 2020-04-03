#include <agz/tracer/utility/reflection.h>

#include "./phong_specular_comp.h"

AGZ_TRACER_BEGIN

Vec3 PhongSpecularComponent::sample_pow_cos_on_hemisphere(
    real e, const Sample2 &sam) const noexcept
{
    const real cos_theta_h = std::pow(sam.u, 1 / (e + 1));
    const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
    const real phi = 2 * PI_r * sam.v;

    return Vec3(
        sin_theta_h * std::cos(phi),
        sin_theta_h * std::sin(phi),
        cos_theta_h).normalize();
}

real PhongSpecularComponent::pow_cos_on_hemisphere_pdf(real e, real cos_theta) const noexcept
{
    return (e + 1) / (2 * PI_r) * std::pow(cos_theta, e);
}

PhongSpecularComponent::PhongSpecularComponent(const Spectrum &s, real ns) noexcept
    : BSDFComponent(BSDF_GLOSSY)
{
    s_ = s;
    ns_ = ns;
}

Spectrum PhongSpecularComponent::eval(
    const Vec3 &lwi, const Vec3 &lwo,
    TransMode mode) const noexcept
{
    if(lwi.z <= 0 || lwo.z <= 0)
        return {};

    const Vec3 ideal_lwi = refl_aux::reflect(lwo, { 0, 0, 1 });
    const real cos_val = cos(ideal_lwi, lwi);
    return s_ * pow_cos_on_hemisphere_pdf(ns_, cos_val);
}

PhongSpecularComponent::SampleResult PhongSpecularComponent::sample(
    const Vec3 &lwo, TransMode mode,
    const Sample2 &sam) const noexcept
{
    if(lwo.z <= 0)
        return {};
    
    const Vec3 ideal_lwi = refl_aux::reflect(lwo, { 0, 0, 1 });
    const Vec3 local_lwi = sample_pow_cos_on_hemisphere(ns_, sam);
    const Vec3 lwi = Coord::from_z(ideal_lwi).local_to_global(
        local_lwi).normalize();
    if(lwi.z <= 0)
        return {};

    const real pdf = pow_cos_on_hemisphere_pdf(ns_, local_lwi.z);

    SampleResult ret;
    ret.f   = eval(lwi, lwo, mode);
    ret.lwi = lwi;
    ret.pdf = pdf;

    return ret;
}

real PhongSpecularComponent::pdf(
    const Vec3 &lwi, const Vec3 &lwo) const noexcept
{
    if(lwi.z <= 0 || lwo.z <= 0)
        return 0;

    const Vec3 ideal_lwi = refl_aux::reflect(lwo, { 0, 0, 1 });
    const Vec3 local_lwi = Coord::from_z(ideal_lwi).global_to_local(lwi);

    if(local_lwi.z <= 0)
        return 0;
    return pow_cos_on_hemisphere_pdf(ns_, local_lwi.normalize().z);
}

AGZ_TRACER_END
