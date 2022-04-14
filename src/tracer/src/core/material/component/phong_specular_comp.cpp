#include <agz/tracer/utility/reflection.h>

#include "./phong_specular_comp.h"

AGZ_TRACER_BEGIN

FVec3 PhongSpecularComponent::sample_pow_cos_on_hemisphere(real e, const Sample2 &sam) const
{
    const real cos_theta_h = std::pow(sam.u, 1 / (e + 1));
    const real sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
    const real phi = 2 * PI_r * sam.v;

    return FVec3(
        sin_theta_h * std::cos(phi),
        sin_theta_h * std::sin(phi),
        cos_theta_h).normalize();
}

real PhongSpecularComponent::pow_cos_on_hemisphere_pdf(real e, real cos_theta) const
{
    return (e + 1) / (2 * PI_r) * std::pow(cos_theta, e);
}

PhongSpecularComponent::PhongSpecularComponent(const FSpectrum &s, real ns) noexcept
{
    s_ = s;
    ns_ = ns;
}

FSpectrum PhongSpecularComponent::eval(const FVec3 &lwi, const FVec3 &lwo, TransMode mode) const
{
    if(lwi.z <= 0 || lwo.z <= 0)
        return {};

    const FVec3 ideal_lwi = refl_aux::reflect(lwo, { 0, 0, 1 });
    const real cos_val = cos(ideal_lwi, lwi);
    return s_ * pow_cos_on_hemisphere_pdf(ns_, cos_val);
}

PhongSpecularComponent::SampleResult PhongSpecularComponent::sample(
    const FVec3 &lwo, TransMode mode, const Sample2 &sam) const
{
    if(lwo.z <= 0)
        return {};
    
    const FVec3 ideal_lwi = refl_aux::reflect(lwo, { 0, 0, 1 });
    const FVec3 local_lwi = sample_pow_cos_on_hemisphere(ns_, sam);
    const FVec3 lwi = FCoord::from_z(ideal_lwi).local_to_global(
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

BSDFComponent::BidirSampleResult PhongSpecularComponent::sample_bidir(
    const FVec3 &lwo, TransMode mode, const Sample2 &sam) const
{
    auto t = sample(lwo, mode, sam);
    BidirSampleResult ret = {};
    if(t.is_valid())
    {
        ret.lwi = t.lwi;
        ret.f = t.f;
        ret.pdf = t.pdf;
        ret.pdf_rev = pdf(lwo, ret.lwi);
    }
    return ret;
}

real PhongSpecularComponent::pdf(const FVec3 &lwi, const FVec3 &lwo) const
{
    if(lwi.z <= 0 || lwo.z <= 0)
        return 0;

    const FVec3 ideal_lwi = refl_aux::reflect(lwo, { 0, 0, 1 });
    const FVec3 local_lwi = FCoord::from_z(ideal_lwi).global_to_local(lwi);

    if(local_lwi.z <= 0)
        return 0;
    return pow_cos_on_hemisphere_pdf(ns_, local_lwi.normalize().z);
}

bool PhongSpecularComponent::has_diffuse_component() const
{
    return false;
}

AGZ_TRACER_END
