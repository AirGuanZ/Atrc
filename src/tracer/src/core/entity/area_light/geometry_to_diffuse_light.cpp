#include "./geometry_to_diffuse_light.h"

AGZ_TRACER_BEGIN

GeometryToDiffuseLight::GeometryToDiffuseLight(const Geometry *geometry, const Spectrum &radiance)
    : geometry_(geometry), radiance_(radiance)
{
    
}

LightSampleResult GeometryToDiffuseLight::sample(const Vec3 &ref, const Sample5 &sam) const noexcept
{
    real pdf_area;
    auto spt = geometry_->sample(ref, &pdf_area, { sam.u, sam.v, sam.r });

    if(dot(spt.geometry_coord.z, ref - spt.pos) <= 0)
        return LIGHT_SAMPLE_RESULT_NULL;

    const Vec3 spt_to_ref = ref - spt.pos;
    const real dist2 = spt_to_ref.length_square();

    LightSampleResult ret;
    ret.pos      = spt.pos;
    ret.nor      = spt.geometry_coord.z;
    ret.ref      = ref;
    ret.radiance = radiance_;
    ret.pdf      = pdf_area * dist2 / std::abs(cos(spt.geometry_coord.z, spt_to_ref));

    return ret;
}

LightEmitResult GeometryToDiffuseLight::sample_emit(const Sample5 &sam) const noexcept
{
    real pdf_pos;
    const auto surface_point = geometry_->sample(&pdf_pos, { sam.u, sam.v, sam.w });
    
    const auto [local_dir, pdf_dir] = math::distribution::zweighted_on_hemisphere(sam.r, sam.s);
    const Vec3 global_dir = surface_point.geometry_coord.local_to_global(local_dir);
    
    LightEmitResult ret;
    ret.pos       = surface_point.eps_offset(global_dir);
    ret.dir       = global_dir;
    ret.nor       = surface_point.geometry_coord.z;
    ret.radiance  = radiance_;
    ret.pdf_pos   = pdf_pos;
    ret.pdf_dir   = pdf_dir;
    
    return ret;
}

LightEmitPDFResult GeometryToDiffuseLight::emit_pdf(const Vec3 &position, const Vec3 &direction, const Vec3 &normal) const noexcept
{
    const real local_dir_z = cos(direction, normal);
    const real pdf_pos = geometry_->pdf(position);
    const real pdf_dir = math::distribution::zweighted_on_hemisphere_pdf(local_dir_z);
    return { pdf_pos, pdf_dir };
}

Spectrum GeometryToDiffuseLight::power() const noexcept
{
    return PI_r * radiance_ * geometry_->surface_area();
}

Spectrum GeometryToDiffuseLight::radiance(const Vec3 &pos, const Vec3 &nor, const Vec2 &uv, const Vec3 &light_to_out) const noexcept
{
    return dot(nor, light_to_out) > 0 ? radiance_ : Spectrum();
}

real GeometryToDiffuseLight::pdf(const Vec3 &ref, const SurfacePoint &spt) const noexcept
{
    if(dot(spt.geometry_coord.z, ref - spt.pos) <= 0)
        return 0;
    
    const real area_pdf = geometry_->pdf(ref, spt.pos);
    const Vec3 spt_to_ref = ref - spt.pos;
    const real dist2 = spt_to_ref.length_square();
    const real area_to_solid_angle_factor = dist2 / std::abs(cos(spt.geometry_coord.z, spt_to_ref));
    return area_pdf * area_to_solid_angle_factor;
}

AGZ_TRACER_END
