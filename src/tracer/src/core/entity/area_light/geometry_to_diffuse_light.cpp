#include "./geometry_to_diffuse_light.h"

AGZ_TRACER_BEGIN

GeometryToDiffuseLight::GeometryToDiffuseLight(
    const Geometry *geometry, const FSpectrum &radiance,
    real user_specified_power)
    : geometry_(geometry), radiance_(radiance),
      user_specified_power_(user_specified_power)
{
    
}

LightSampleResult GeometryToDiffuseLight::sample(
    const FVec3 &ref, const Sample5 &sam) const noexcept
{
    real pdf_area;
    auto spt = geometry_->sample(ref, &pdf_area, { sam.u, sam.v, sam.r });

    if(dot(spt.geometry_coord.z, ref - spt.pos) <= 0)
        return LIGHT_SAMPLE_RESULT_NULL;

    const FVec3 spt_to_ref = ref - spt.pos;
    const real dist2 = spt_to_ref.length_square();

    const real pdf = pdf_area * dist2
                   / std::abs(cos(spt.geometry_coord.z, spt_to_ref));

    return LightSampleResult(
        ref, spt.pos, spt.geometry_coord.z, radiance_, pdf);
}

LightEmitResult GeometryToDiffuseLight::sample_emit(
    const Sample5 &sam) const noexcept
{
    real pdf_pos;
    const auto surface_point = geometry_->sample(
        &pdf_pos, { sam.u, sam.v, sam.w });
    
    const auto [local_dir, pdf_dir] = math::distribution
                                        ::zweighted_on_hemisphere(sam.r, sam.s);

    const FVec3 dir = surface_point.geometry_coord.local_to_global(local_dir);
    
    return LightEmitResult(
        surface_point.eps_offset(dir), dir,
        surface_point.geometry_coord.z, surface_point.uv,
        radiance_, pdf_pos, pdf_dir);
}

LightEmitPDFResult GeometryToDiffuseLight::emit_pdf(
    const FVec3 &pos, const FVec3 &dir, const FVec3 &nor) const noexcept
{
    const real local_dir_z = cos(dir, nor);
    const real pdf_pos = geometry_->pdf(pos);
    const real pdf_dir = math::distribution
                            ::zweighted_on_hemisphere_pdf(local_dir_z);
    return { pdf_pos, pdf_dir };
}

FSpectrum GeometryToDiffuseLight::power() const noexcept
{
    if(user_specified_power_ > 0)
        return FSpectrum(user_specified_power_);
    return PI_r * radiance_ * geometry_->surface_area();
}

FSpectrum GeometryToDiffuseLight::radiance(
    const FVec3 &pos, const FVec3 &nor, const Vec2 &uv,
    const FVec3 &light_to_out) const noexcept
{
    return dot(nor, light_to_out) > 0 ? radiance_ : FSpectrum();
}

real GeometryToDiffuseLight::pdf(
    const FVec3 &ref,
    const FVec3 &pos, const FVec3 &nor) const noexcept
{
    if(dot(nor, ref - pos) <= 0)
        return 0;
    
    const real area_pdf = geometry_->pdf(ref, pos);
    
    const FVec3 spt_to_ref = ref - pos;
    const real dist2 = spt_to_ref.length_square();
    const real abscos = std::abs(cos(nor, spt_to_ref));
    const real area_to_solid_angle_factor = dist2 / abscos;
    
    return area_pdf * area_to_solid_angle_factor;
}

AGZ_TRACER_END
