#pragma once

#include <cstdint>

#include <agz/tracer_utility/config.h>

/*
CShader的完整实现由三部分构成：
    1. Tracer部分的CMaterial，作为Tracer方面实现了Material的接口
    2. Tracer_CShader部分，将通过C接口部分得到的数据转发给Shader
    3. Shader部分，由用户用C++实现的材质
1内建于Tracer内部，2同样是Atrc的一部分，以静态库的形式村子啊，3完全由用户编写。
用户自行将2、3链接得到动态库，由1加载之，1和2通过C接口通信，2将C接口翻译成C++给3使用

下面规定了1、2间通信用的C接口
*/

extern "C"
{

typedef agz::tracer::real agzt_real;

struct AGZTSpectrum
{
    agzt_real r;
    agzt_real g;
    agzt_real b;
};

struct AGZTVec2
{
    agzt_real x;
    agzt_real y;
};

struct AGZTVec3
{
    agzt_real x;
    agzt_real y;
    agzt_real z;
};

struct AGZTCoord
{
    AGZTVec3 x;
    AGZTVec3 y;
    AGZTVec3 z;
};

struct AGZTSample3
{
    agzt_real u;
    agzt_real v;
    agzt_real w;
};

struct AGZTIntersection
{
    AGZTVec3 pos;
    AGZTVec2 uv;
    AGZTCoord geometry_coord;
    AGZTCoord user_coord;
    agzt_real t;
    AGZTVec3 wr;
};

struct AGZTConfigNode;
struct AGZTConfigGroup;
struct AGZTConfigArray;
struct AGZTConfigValue;

enum class AGZTConfigNodeType
{
    Group = 0, Array = 1, Value = 2
};

struct AGZTConfigGroup
{
    size_t size;
    const char *const*keys;
    const AGZTConfigNode *nodes;
};

struct AGZTConfigArray
{
    size_t size;
    const AGZTConfigNode *elems;
};

struct AGZTConfigValue
{
    const char *str;
};

struct AGZTConfigNode
{
    AGZTConfigNodeType type;
    union
    {
        AGZTConfigGroup group;
        AGZTConfigArray array;
        AGZTConfigValue value;
    };
};

// Tracer通过以下函数来和shader插件交互

typedef void* AGZT_COperationHandler;

typedef void* AGZT_FresnelHandle;
typedef void* AGZT_FresnelPointHandle;
typedef void *AGZT_TextureHandle;

typedef void* AGZT_MaterialHandle;
typedef void* AGZT_BSDFHandle;

typedef void* AGZT_InitContextHandle;
typedef void* AGZT_ArenaHandle;

typedef AGZT_FresnelHandle     (*AGZT_CreateFresnel_FuncPtr)     (const AGZTConfigGroup*, AGZT_InitContextHandle init_ctx);
typedef AGZT_FresnelPointHandle(*AGZT_CreateFresnelPoint_FuncPtr)(AGZT_FresnelHandle, const AGZTVec2 *uv, AGZT_ArenaHandle arena);
typedef AGZTSpectrum           (*AGZT_EvalFresnelPoint_FuncPtr)  (AGZT_FresnelPointHandle, agzt_real cos_theta_i);
typedef agzt_real              (*AGZT_GetFresnelPointEtaI)       (AGZT_FresnelPointHandle);
typedef agzt_real              (*AGZT_GetFresnelPointEtaO)       (AGZT_FresnelPointHandle);

typedef AGZT_TextureHandle(*AGZT_CreateTexture_FuncPtr)        (const AGZTConfigGroup*, AGZT_InitContextHandle init_ctx);
typedef agzt_real         (*AGZT_SampleTextureReal_FuncPtr)    (AGZT_TextureHandle, const AGZTVec2*);
typedef AGZTSpectrum      (*AGZT_SampleTextureSpectrum_FuncPtr)(AGZT_TextureHandle, const AGZTVec2*);

// 2通过C接口能让1做的事情
struct AGZT_COperations
{
    // 创建一个fresnel对象，返回其句柄
    AGZT_CreateFresnel_FuncPtr create_fresnel;
    
    // 让指定的fresnel对象产生一个fresnel point
    AGZT_CreateFresnelPoint_FuncPtr create_fresnel_point;
    
    // 用指定的frensel point求特定入射角处的fresnel值
    AGZT_EvalFresnelPoint_FuncPtr eval_fresnel_point;

    // 取得某个fresnel point的内折射率
    AGZT_GetFresnelPointEtaI get_fresnel_eta_i;

    // 取得某个fresnel point的外折射率
    AGZT_GetFresnelPointEtaO get_fresnel_eta_o;

    // 创建一个纹理对象，返回其句柄
    AGZT_CreateTexture_FuncPtr create_texture;
    
    // 采样指定的纹理对象，得到一个浮点数
    AGZT_SampleTextureReal_FuncPtr sample_real;
    
    // 采样指定的纹理对象，得到一个spectrum
    AGZT_SampleTextureSpectrum_FuncPtr sample_spectrum;
};

// global function "create_material"
typedef AGZT_MaterialHandle(*AGZT_CreateMaterial_FuncPtr)(const AGZTConfigGroup *params, const AGZT_COperations *oprs, AGZT_InitContextHandle init_ctx);

// global function "destroy_material"
typedef void(*AGZT_DestroyMaterial_FuncPtr)(AGZT_MaterialHandle material_handle);

// global function "create_bsdf"
typedef AGZT_BSDFHandle(*AGZT_CreateBSDF_FuncPtr)(AGZT_MaterialHandle material_handle, const AGZTIntersection *intity, AGZT_ArenaHandle arena);

// global function "destroy_bsdf"
typedef void(*AGZT_DestroyBSDF_FuncPtr)(AGZT_BSDFHandle bsdf_handle);

// global function "bsdf_eval"
typedef AGZTSpectrum(*AGZT_BSDF_Eval_FuncPtr)(AGZT_BSDFHandle bsdf_handle, const AGZTVec3 *wi, const AGZTVec3 *wo, bool is_importance);

// global function "bsdf_sample"
typedef void(*AGZT_BSDF_Sample_FuncPtr)(AGZT_BSDFHandle bsdf_handle, const AGZTVec3 *wo, bool is_importance, const AGZTSample3 *sam,
                                        AGZTVec3 *out_dir, AGZTSpectrum *out_f, agzt_real *out_pdf, bool *out_is_importance, bool *out_is_delta);

// global function "bsdf_pdf"
typedef agzt_real(*AGZT_BSDF_PDF_FuncPtr)(AGZT_BSDFHandle bsdf_handle, const AGZTVec3 *wi, const AGZTVec3 *wo, bool is_importance);

// global function "bsdf_albedo"
typedef AGZTSpectrum(*AGZT_BSDF_Albedo_FuncPtr)(AGZT_BSDFHandle bsdf_handle);

// global function "bsdf_is_delta"
typedef bool(*AGZT_BSDF_IsDelta_FuncPtr)(AGZT_BSDFHandle bsdf_handle);

// global function "bsdf_is_black"
typedef bool(*AGZT_BSDF_IsBlack_FuncPtr)(AGZT_BSDFHandle bsdf_handle);

} // extern "C"

AGZ_TRACER_BEGIN

void cpp_to_c(const ConfigNode &node, Arena &arena, AGZTConfigNode *output);
void cpp_to_c(const ConfigGroup &group, Arena &arena, AGZTConfigGroup *output);
void cpp_to_c(const ConfigArray &array, Arena &arena, AGZTConfigArray *output);
void cpp_to_c(const ConfigValue &value, Arena &arena, AGZTConfigValue *output);

std::shared_ptr<ConfigNode>  c_to_cpp(const AGZTConfigNode *node);
std::shared_ptr<ConfigGroup> c_to_cpp(const AGZTConfigGroup *group);
std::shared_ptr<ConfigArray> c_to_cpp(const AGZTConfigArray *array);
std::shared_ptr<ConfigValue> c_to_cpp(const AGZTConfigValue *value);

inline AGZTVec2     cpp_to_c(const Vec2 &v)      { return { v.x, v.y }; }
inline AGZTVec3     cpp_to_c(const Vec3 &v)      { return { v.x, v.y, v.z }; }
inline AGZTCoord    cpp_to_c(const Coord &c)     { return { cpp_to_c(c.x), cpp_to_c(c.y), cpp_to_c(c.z) }; }
inline AGZTSample3  cpp_to_c(const Sample3 &sam) { return { sam.u, sam.v, sam.w }; }
inline AGZTSpectrum cpp_to_c(const Spectrum &s)  { return { s.r, s.g, s.b }; }

inline Vec2     c_to_cpp(const AGZTVec2 &v)      { return { v.x, v.y }; }
inline Vec3     c_to_cpp(const AGZTVec3 &v)      { return { v.x, v.y, v.z }; }
inline Spectrum c_to_cpp(const AGZTSpectrum &s)  { return { s.r, s.g, s.b }; }
inline Coord    c_to_cpp(const AGZTCoord &c)     { return { c_to_cpp(c.x), c_to_cpp(c.y), c_to_cpp(c.z) }; }
inline Sample3  c_to_cpp(const AGZTSample3 &sam) { return { sam.u, sam.v, sam.w }; }

AGZ_TRACER_END
