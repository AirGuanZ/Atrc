#pragma once

#include <agz/tracer_utility/math.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

enum TransportMode
{
    TM_Radiance = 0,   // camera -> light
    TM_Importance = 1  // light -> camera
};

struct BSDFSampleResult
{
    Vec3          dir;      // 采样得到的散射方向
    Spectrum      f;        // bsdf值
    real          pdf;      // 采样的概率密度函数值,w.r.t. solid angle
    TransportMode mode;     // 传输模式
    bool          is_delta; // pdf和f是否是delta函数

    bool invalid() const noexcept
    {
        return !dir;
    }
};

inline BSDFSampleResult BSDF_SAMPLE_RESULT_INVALID = { {}, {}, 0, TM_Radiance, false };

/**
 * @brief 双向散射分布函数（bidirectional scattering distribution function）接口
 *
 * f(x, in_dir, out_dir)中的x被预先给定，故实际上只有两个参数：in_dir和out_dir
 */
class BSDF : public misc::uncopyable_t
{
public:

    virtual ~BSDF() = default;

    virtual Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept = 0;

    virtual BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept = 0;

    virtual real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept = 0;

    virtual Spectrum albedo() const noexcept = 0;

    virtual bool is_delta() const noexcept { return false; }

    virtual bool is_black() const noexcept { return false; }
};

/**
 * @brief 定义了x点的局部坐标系的BSDF实现
 */
class LocalBSDF : public BSDF
{
protected:

    Coord geometry_coord_;
    Coord shading_coord_;

    // w位于shading_coord和geometry_coord的z=0平面的夹缝中间的情况
    // 此时bsdf是未定义的
    bool cause_black_fringes(const Vec3 &w) const noexcept
    {
        bool shading_posi  = shading_coord_.in_positive_z_hemisphere(w);
        bool geometry_posi = geometry_coord_.in_positive_z_hemisphere(w);
        return shading_posi != geometry_posi;
    }

    bool cause_black_fringes(const Vec3 &w1, const Vec3 &w2) const noexcept
    {
        return cause_black_fringes(w1) || cause_black_fringes(w2);
    }

    Spectrum eval_for_black_fringes(const Vec3 &in, const Vec3 &out) const noexcept
    {
        if(!geometry_coord_.in_positive_z_hemisphere(in) ||
           !geometry_coord_.in_positive_z_hemisphere(out))
            return Spectrum();
        return albedo() / PI_r;
    }

    BSDFSampleResult sample_for_black_fringes(const Vec3 &out, TransportMode mode, const Sample3 &sam) const noexcept
    {
        if(!geometry_coord_.in_positive_z_hemisphere(out))
            return BSDF_SAMPLE_RESULT_INVALID;

        auto [local_in, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
        if(pdf < EPS)
            return BSDF_SAMPLE_RESULT_INVALID;

        BSDFSampleResult ret;
        ret.dir      = geometry_coord_.local_to_global(local_in).normalize();
        ret.f        = albedo() / PI_r * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
        ret.pdf      = pdf;
        ret.mode     = mode;
        ret.is_delta = false;

        return ret;
    }

    real pdf_for_black_fringes(const Vec3 &in, const Vec3 &out) const noexcept
    {
        if(geometry_coord_.in_positive_z_hemisphere(in) != geometry_coord_.in_positive_z_hemisphere(out))
            return false;
        Vec3 local_in = geometry_coord_.global_to_local(in).normalize();
        return math::distribution::zweighted_on_hemisphere_pdf(local_in.z);
    }

public:

    /**
     * @param geometry_coord 几何局部坐标系，由物体的数学正确的几何属性确定
     * @param shading_coord 着色局部坐标系，在几何坐标系的基础上经法线插值、纹理扰动等处理得来
     */
    LocalBSDF(const Coord &geometry_coord, const Coord &shading_coord) noexcept
        : geometry_coord_(geometry_coord), shading_coord_(shading_coord)
    {

    }
};

/**
 * @brief 局部坐标系内的BSDF实现，其in_dir和out_dir默认位于local coord中且已归一化
 */
class InternalBSDF : public BSDF
{
public:

    using BSDF::BSDF;
};

AGZ_TRACER_END
