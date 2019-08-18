#include <agz/tracer_cshader/c_interface.h>

#ifdef _WIN32
#  define AGZT_DLL_EXPORTIT __declspec( dllexport )
#else
#  define AGZT_DLL_EXPORTIT
#endif

extern "C"
{

AGZT_DLL_EXPORTIT AGZT_MaterialHandle create_material(const AGZTConfigGroup *params, AGZT_COperations *opr)
{
    auto cpp_params = agz::tracer::c_to_cpp(params);
    auto material = agz::tracer::new_material();
    material->_set_c_oprs(opr);
    try
    {
        material->initialize(*cpp_params);
    }
    catch(...)
    {
        return nullptr;
    }
    return material;
}

AGZT_DLL_EXPORTIT void destroy_material(AGZT_MaterialHandle handle)
{
    if(handle)
        delete static_cast<agz::tracer::CShaderMaterial*>(handle);
}

AGZT_DLL_EXPORTIT AGZT_BSDFHandle create_bsdf(AGZT_MaterialHandle material, const AGZTIntersection *inct)
{
    agz::tracer::CShaderMaterial::Intersection t_inct;
    t_inct.pos            = agz::tracer::c_to_cpp(inct->pos);
    t_inct.uv             = agz::tracer::c_to_cpp(inct->uv);
    t_inct.geometry_coord = agz::tracer::c_to_cpp(inct->geometry_coord);
    t_inct.user_coord     = agz::tracer::c_to_cpp(inct->user_coord);
    t_inct.t              = inct->t;
    t_inct.wr             = agz::tracer::c_to_cpp(inct->wr);
    return static_cast<agz::tracer::CShaderMaterial*>(material)
        ->shade(t_inct);
}

AGZT_DLL_EXPORTIT void destroy_bsdf(AGZT_BSDFHandle handle)
{
    if(handle)
        delete static_cast<agz::tracer::CShaderBSDF*>(handle);
}

AGZT_DLL_EXPORTIT AGZTSpectrum bsdf_eval(AGZT_BSDFHandle handle, const AGZTVec3 *cwi, const AGZTVec3 *cwo, bool is_importance)
{
    auto wi = agz::tracer::c_to_cpp(*cwi);
    auto wo = agz::tracer::c_to_cpp(*cwo);
    auto bsdf = static_cast<agz::tracer::CShaderBSDF*>(handle);
    auto spec = bsdf->eval(wi, wo, is_importance);
    return agz::tracer::cpp_to_c(spec);
}

AGZT_DLL_EXPORTIT agzt_real bsdf_proj_wi_factor(AGZT_BSDFHandle handle, const AGZTVec3 *cwi)
{
    auto wi = agz::tracer::c_to_cpp(*cwi);
    auto bsdf = static_cast<agz::tracer::CShaderBSDF*>(handle);
    return bsdf->proj_wi_factor(wi);
}

AGZT_DLL_EXPORTIT void bsdf_sample(AGZT_BSDFHandle handle, const AGZTVec3 *cwo, bool is_importance, const AGZTSample3 *csam,
                                   AGZTVec3 *out_dir, AGZTSpectrum *out_f, agzt_real *out_pdf, bool *out_is_importance, bool *out_is_delta)
{
    auto wo = agz::tracer::c_to_cpp(*cwo);
    auto sam = agz::tracer::c_to_cpp(*csam);
    auto bsdf = static_cast<agz::tracer::CShaderBSDF*>(handle);

    auto result = bsdf->sample(wo, is_importance, sam);

    *out_dir           = agz::tracer::cpp_to_c(result.dir);
    *out_f             = agz::tracer::cpp_to_c(result.f);
    *out_pdf           = result.pdf;
    *out_is_importance = result.is_importance;
    *out_is_delta      = result.is_delta;
}

AGZT_DLL_EXPORTIT agzt_real bsdf_pdf(AGZT_BSDFHandle handle, const AGZTVec3 *cwi, const AGZTVec3 *cwo, bool is_importance)
{
    auto wi = agz::tracer::c_to_cpp(*cwi);
    auto wo = agz::tracer::c_to_cpp(*cwo);
    auto bsdf = static_cast<agz::tracer::CShaderBSDF*>(handle);
    return bsdf->pdf(wi, wo, is_importance);
}

AGZT_DLL_EXPORTIT AGZTSpectrum bsdf_albedo(AGZT_BSDFHandle handle)
{
    auto c_spec = static_cast<agz::tracer::CShaderBSDF*>(handle)->albedo();
    return agz::tracer::cpp_to_c(c_spec);
}

AGZT_DLL_EXPORTIT bool bsdf_is_delta(AGZT_BSDFHandle handle)
{
    return static_cast<agz::tracer::CShaderBSDF*>(handle)->is_delta();
}

AGZT_DLL_EXPORTIT bool bsdf_is_black(AGZT_BSDFHandle handle)
{
    return static_cast<agz::tracer::CShaderBSDF*>(handle)->is_black();
}

} // extern "C"

AGZTTextureHandle agz::tracer::CShaderMaterial::create_texture(const ConfigGroup &params)
{
    AGZTConfigGroup c_params = {}; Arena arena;
    cpp_to_c(params, arena, &c_params);
    return c_oprs_->create_texture(c_oprs_->operation_handler, &c_params);
}

agz::tracer::real agz::tracer::CShaderMaterial::sample_real(AGZTTextureHandle tex, const Vec2 &uv) const
{
    auto c_uv = cpp_to_c(uv);
    return c_oprs_->sample_real(c_oprs_->operation_handler, tex, &c_uv);
}

agz::tracer::Spectrum agz::tracer::CShaderMaterial::sample_spectrum(AGZTTextureHandle tex, const Vec2 &uv) const
{
    auto c_uv = cpp_to_c(uv);
    auto c_spec = c_oprs_->sample_spectrum(c_oprs_->operation_handler, tex, &c_uv);
    return c_to_cpp(c_spec);
}
