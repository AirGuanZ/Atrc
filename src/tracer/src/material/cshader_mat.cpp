#include <filesystem>
#include <map>
#include <memory>
#include <string>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include <agz/tracer/utility/logger.h>
#include <agz/tracer_utility/shader_c_api.h>
#include <agz/utility/system/library.h>

#include "./ideal_black.h"

extern "C"
{

AGZTTextureHandle agzt_create_texture_impl(AGZT_COperationHandler handler, const AGZTConfigGroup *params);

agzt_real agzt_sample_texture_real_impl(AGZT_COperationHandler handler, AGZTTextureHandle tex, const AGZTVec2 *uv);

AGZTSpectrum agzt_sample_texture_spectrum_impl(AGZT_COperationHandler handler, AGZTTextureHandle tex, const AGZTVec2 *uv);

} // extern "C"

AGZ_TRACER_BEGIN

namespace
{
    struct CAPI
    {
        AGZT_CreateMaterial_FuncPtr  create_material;
        AGZT_DestroyMaterial_FuncPtr destroy_material;

        AGZT_CreateBSDF_FuncPtr  create_bsdf;
        AGZT_DestroyBSDF_FuncPtr destroy_bsdf;

        AGZT_BSDF_Eval_FuncPtr         bsdf_eval;
        AGZT_BSDF_ProjWiFactor_FuncPtr bsdf_proj_wi_factor;
        AGZT_BSDF_Sample_FuncPtr       bsdf_sample;
        AGZT_BSDF_PDF_FuncPtr          bsdf_pdf;
        AGZT_BSDF_Albedo_FuncPtr       bsdf_albedo;
        AGZT_BSDF_IsDelta_FuncPtr      bsdf_is_delta;
        AGZT_BSDF_IsBlack_FuncPtr      bsdf_is_black;
    };

    CAPI load_c_api(const sys::shared_lib_t &lib)
    {
        CAPI ret;

#define GET_METHOD(name) \
        do { \
            ret.name = lib.get_proc<decltype(ret.name)>(#name); \
            if(!ret.name) \
                throw ObjectConstructionException("failed to get proc '" + std::string(#name) + "' from material library"); \
        } while(false)

        GET_METHOD(create_material);
        GET_METHOD(destroy_material);
        GET_METHOD(create_bsdf);
        GET_METHOD(destroy_bsdf);

        GET_METHOD(bsdf_eval);
        GET_METHOD(bsdf_proj_wi_factor);
        GET_METHOD(bsdf_sample);
        GET_METHOD(bsdf_pdf);
        GET_METHOD(bsdf_albedo);
        GET_METHOD(bsdf_is_delta);
        GET_METHOD(bsdf_is_black);

#undef GET_METHOD

        return ret;
    }

    std::string shared_lib_filename(const std::string &raw)
    {
        if(std::filesystem::exists(raw))
            return raw;
#ifdef AGZ_OS_WIN32
        if(std::filesystem::exists(raw + ".dll"))
            return raw + ".dll";
#else
        if(std::filesystem::exists(raw + ".so"))
            return raw + ".so";
        if(std::filesystem::exists("lib" + raw))
            return "lib" + raw;
        if(std::filesystem::exists("lib" + raw + "so"))
            return "lib" + raw + "so";
#endif
        return raw;
    }

    sys::shared_lib_t *load_dynamic_library(const std::string &raw_filename)
    {
        std::string filename = shared_lib_filename(raw_filename);

        static std::map<std::string, std::unique_ptr<sys::shared_lib_t>> filename2lib;
        auto it = filename2lib.find(filename);
        if(it != filename2lib.end())
            return it->second.get();

        auto lib = std::make_unique<sys::shared_lib_t>(filename);
        auto ret = lib.get();
        filename2lib[filename] = std::move(lib);

        AGZ_LOG1("load material from shared library " + filename);

        return ret;
    }

    class CShaderBSDF : public BSDF
    {
        AGZT_BSDFHandle bsdf_handle_;
        const CAPI *c_api_;

    public:

        CShaderBSDF(AGZT_BSDFHandle handle, const CAPI *c_api)
            : bsdf_handle_(handle), c_api_(c_api)
        {
            assert(handle && c_api);
        }

        ~CShaderBSDF()
        {
            if(bsdf_handle_)
                c_api_->destroy_bsdf(bsdf_handle_);
        }

        Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            auto cwi = cpp_to_c(wi), cwo = cpp_to_c(wo);
            auto c_spectrum = c_api_->bsdf_eval(bsdf_handle_, &cwi, &cwo, mode == TM_Importance);
            return c_to_cpp(c_spectrum);
        }

        real proj_wi_factor(const Vec3 &wi) const noexcept override
        {
            auto cwi = cpp_to_c(wi);
            return c_api_->bsdf_proj_wi_factor(bsdf_handle_, &cwi);
        }

        BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
        {
            auto cwo = cpp_to_c(wo);
            bool is_importance = mode == TM_Importance;
            auto csam = cpp_to_c(sam);

            AGZTVec3     out_dir = {};
            AGZTSpectrum out_f   = {};
            real         out_pdf;
            bool         out_is_importance;
            bool         out_is_delta;
            c_api_->bsdf_sample(bsdf_handle_, &cwo, is_importance, &csam,
                                &out_dir, &out_f, &out_pdf, &out_is_importance, &out_is_delta);

            BSDFSampleResult ret;
            ret.dir      = c_to_cpp(out_dir);
            ret.f        = c_to_cpp(out_f);
            ret.pdf      = out_pdf;
            ret.mode     = out_is_importance ? TM_Importance : TM_Radiance;
            ret.is_delta = out_is_delta;

            return ret;
        }

        real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            auto cwi = cpp_to_c(wi), cwo = cpp_to_c(wo);
            return c_api_->bsdf_pdf(bsdf_handle_, &cwi, &cwo, mode == TM_Importance);
        }

        Spectrum albedo() const noexcept override
        {
            auto c_spec = c_api_->bsdf_albedo(bsdf_handle_);
            return c_to_cpp(c_spec);
        }

        bool is_delta() const noexcept override
        {
            return c_api_->bsdf_is_delta(bsdf_handle_);
        }

        bool is_black() const noexcept override
        {
            return c_api_->bsdf_is_black(bsdf_handle_);
        }
    };
}

class CShaderMaterial : public Material
{
    AGZT_MaterialHandle material_handle_ = nullptr;
    CAPI c_api_ = {};

    obj::ObjectInitContext *internal_init_ctx_ = nullptr;
    std::vector<const Texture*> textures_;

    AGZT_COperations c_oprs_ = {};

public:

    using Material::Material;

    ~CShaderMaterial()
    {
        if(c_api_.destroy_material && material_handle_)
            c_api_.destroy_material(material_handle_);
    }

    static std::string description()
    {
        return R"___(
lib [Material]
    filename [string] dynamic library path
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        internal_init_ctx_ = &init_ctx;

        auto raw_filename = params.child_str("filename");
        auto filename = init_ctx.path_mgr->get(raw_filename);
        auto lib = load_dynamic_library(filename);

        c_api_ = load_c_api(*lib);

        Arena c_params_arena;
        AGZTConfigGroup c_params{};
        cpp_to_c(params, c_params_arena, &c_params);

        c_oprs_.operation_handler = this;
        c_oprs_.create_texture    = agzt_create_texture_impl;
        c_oprs_.sample_real       = agzt_sample_texture_real_impl;
        c_oprs_.sample_spectrum   = agzt_sample_texture_spectrum_impl;
        material_handle_ = c_api_.create_material(&c_params, &c_oprs_);
        if(!material_handle_)
            throw ObjectConstructionException("failed to create dynamic library material handle");

        internal_init_ctx_ = nullptr;

        AGZ_HIERARCHY_WRAP("in initializing dynamic library material")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        AGZTIntersection cinct;
        cinct.pos            = cpp_to_c(inct.pos);
        cinct.uv             = cpp_to_c(inct.uv);
        cinct.geometry_coord = cpp_to_c(inct.geometry_coord);
        cinct.user_coord     = cpp_to_c(inct.user_coord);
        cinct.t              = inct.t;
        cinct.wr             = cpp_to_c(inct.wr);

        auto bsdf_handle = c_api_.create_bsdf(material_handle_, &cinct);
        if(!bsdf_handle)
            return { IdealBlack::IDEAL_BLACK_BSDF_INSTANCE() };

        auto bsdf = arena.create<CShaderBSDF>(bsdf_handle, &c_api_);
        return { bsdf };
    }

    AGZTTextureHandle create_texture(const ConfigGroup &params)
    {
        if(!internal_init_ctx_)
        {
            AGZ_LOG0("invalid initialization");
            return -1;
        }

        try
        {
            AGZTTextureHandle ret = static_cast<int>(textures_.size());
            textures_.push_back(TextureFactory.create(params, *internal_init_ctx_));
            return ret;
        }
        catch(const std::exception &err)
        {
            AGZ_LOG0(err.what());
            return -1;
        }
        catch(...)
        {
            return -1;
        }
    }

    const Texture *get_texture(AGZTTextureHandle tex)
    {
        if(tex < 0 || tex >= static_cast<int>(textures_.size()))
            return nullptr;
        return textures_[tex];
    }
};

AGZT_IMPLEMENTATION(Material, CShaderMaterial, "lib")

AGZ_TRACER_END

extern "C"
{

AGZTTextureHandle agzt_create_texture_impl(AGZT_COperationHandler handler, const AGZTConfigGroup *params)
{
    auto mat = static_cast<agz::tracer::CShaderMaterial*>(handler);
    auto cpp_params = agz::tracer::c_to_cpp(params);
    return mat->create_texture(*cpp_params);
}

agzt_real agzt_sample_texture_real_impl(AGZT_COperationHandler handler, AGZTTextureHandle tex, const AGZTVec2 *uv)
{
    auto mat = static_cast<agz::tracer::CShaderMaterial*>(handler);
    auto texture = mat->get_texture(tex);
    if(!texture)
        return 0;
    return texture->sample_real(agz::tracer::c_to_cpp(*uv));
}

AGZTSpectrum agzt_sample_texture_spectrum_impl(AGZT_COperationHandler handler, AGZTTextureHandle tex, const AGZTVec2 *uv)
{
    auto mat = static_cast<agz::tracer::CShaderMaterial*>(handler);
    auto texture = mat->get_texture(tex);
    if(!texture)
        return { 0, 0, 0 };
    auto spec = texture->sample_spectrum(agz::tracer::c_to_cpp(*uv));
    return agz::tracer::cpp_to_c(spec);
}

} // extern "C"
