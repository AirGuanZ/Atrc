#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>

#include "./component/aggregate.h"
#include "./utility/fresnel_point.h"
#include "./utility/microfacet.h"

AGZ_TRACER_BEGIN

// see A Physically-Based BSDF for Modeling the Appearance of Paper
namespace jensen_paper
{

float ggxRefraction(
    const FVec3 &lwi, const FVec3 &lwo,
    real IOR, real alpha,
    TransMode mode) noexcept
{
    assert(lwi.z * lwo.z < 0);

    const real cos_theta_i = local_angle::cos_theta(lwi);
    const real cos_theta_o = local_angle::cos_theta(lwo);
    const real eta = cos_theta_o > 0 ? IOR : 1 / IOR;

    FVec3 lwh = (lwo + eta * lwi).normalize();
    if(lwh.z < 0)
        lwh = -lwh;

    const real cos_theta_d = dot(lwo, lwh);
    const real F = refl_aux::dielectric_fresnel(IOR, 1, cos_theta_d);

    const real cos_theta_h = local_angle::cos_theta(lwh);
    const real D = microfacet::gtr2(cos_theta_h, alpha);

    const real tan_theta_i = local_angle::tan_theta(lwi);
    const real tan_theta_o = local_angle::tan_theta(lwo);
    const real G = microfacet::smith_gtr2(tan_theta_i, alpha)
                 * microfacet::smith_gtr2(tan_theta_o, alpha);

    const real sdem = cos_theta_d + eta * dot(lwi, lwh);
    const real corr_factor = mode == TransMode::Radiance ? 1 / eta : 1;

    const real val = (1 - F) * D * G * eta * eta
        * dot(lwi, lwh) * dot(lwo, lwh) * corr_factor * corr_factor
        / (cos_theta_i * cos_theta_o * sdem * sdem);

    return std::abs(val);
}

/*
// PrecomputedRhoDtTable is computed with:

void precomputeRhoDtTable()
{
    constexpr int THETA_SIZE = 64;
    constexpr int ETA_SIZE   = 64;
    constexpr int ROUGH_SIZE = 64;
    constexpr int N          = 100000;

    // u: theta:     [0, pi / 2]
    // v: eta:       [1.01, 10]
    // w: roughness: [0, 1]

    for(int i = 0; i < THETA_SIZE; ++i)
    {
        const real theta = PI_r / 2 * (real(i) + real(0.5)) / THETA_SIZE;
        const FVec3 lwi = {
            std::cos(theta) * std::cos(0.0f),
            std::cos(theta) * std::sin(0.0f),
            std::sin(theta)
        };

        for(int j = 0; j < ETA_SIZE; ++j)
        {
            const real eta = (10 - real(1.01)) * (j + real(0.5)) / ETA_SIZE + real(1.01);

            for(int k = 0; k < ROUGH_SIZE; ++k)
            {
                const real roughness = (std::max)(real(0.01), (k + real(0.5)) / ROUGH_SIZE);

                std::default_random_engine rng(42);
                std::uniform_real_distribution<real> dis(0, 1);

                real sum = 0;
                for(int t = 0; t < N; ++t)
                {
                    const real a = dis(rng), b = dis(rng);
                    auto [lwo, pdf] = math::distribution::uniform_on_hemisphere(a, b);

                    const real ft = ggxRefraction(
                        lwi, -lwo, eta, roughness, TransMode::Radiance);
                    sum += ft * std::abs(lwo.z) / pdf;
                }

                const real item = (std::min)(real(1), sum / N);

                print_to_result("real(%.6f), ", item);
            }
            print_to_result("\n");
        }
    }
}*/

class PrecomputedRhoDtTable
{
public:

    static constexpr int THETA_SIZE = 64;
    static constexpr int ETA_SIZE   = 64;
    static constexpr int ROUGH_SIZE = 64;

    PrecomputedRhoDtTable()
    {
        static const real data[] = {
#include "./paper_rho_dt.txt"
        };

        table.initialize(THETA_SIZE, ETA_SIZE, ROUGH_SIZE, data);
    }

    texture::texture3d_t<real> table;

} precomputedRhoDtTable;

class RhoDtTable
{
    real v_;
    real w_;

public:

    RhoDtTable(real eta, real m) noexcept
    {
        v_ = math::saturate((eta - real(1.01)) / (real(10) - real(1.01)));
        w_ = math::saturate(m);
    }

    real get_value(const FVec3 &lwi) const noexcept
    {
        real theta = local_angle::theta(lwi);
        if(theta < 0)
            theta += 2 * PI_r;
        const real u = math::saturate(theta / (2 * PI_r));
        return texture::linear_sample3d(
            Vec3(u, v_, w_),
            [&](int x, int y, int z) { return precomputedRhoDtTable.table(z, y, x); },
            PrecomputedRhoDtTable::ROUGH_SIZE,
            PrecomputedRhoDtTable::ETA_SIZE,
            PrecomputedRhoDtTable::THETA_SIZE);
    }
};

real HG(real cos_io, real g) noexcept
{
    const real g2 = g * g;
    const real dem = 1 + g2 - 2 * g * cos_io;
    return (1 - g2) / (4 * PI_r * dem * std::sqrt(dem));
}

real P(real cos_io, real gf, real gb, real wf, real wb) noexcept
{
    return wf * HG(cos_io, gf) + wb * HG(cos_io, gb);
}

class GGXReflection : public BSDFComponent
{
    FSpectrum color_;

    float alpha_;
    float eta_;

public:

    GGXReflection(const FSpectrum &color, float alpha, float eta)
        : BSDFComponent(BSDF_GLOSSY)
    {
        color_ = color;
        alpha_ = alpha;
        eta_   = eta;
    }

    FSpectrum eval(
        const FVec3 &lwi,
        const FVec3 &lwo,
        TransMode    mode) const noexcept override
    {
        if(lwi.z < EPS() || lwo.z < EPS())
            return {};

        FVec3 lwh = (lwi + lwo).normalize();
        const float D = microfacet::gtr2(local_angle::cos_theta(lwh), alpha_);

        const float F = refl_aux::dielectric_fresnel(
            eta_, 1, local_angle::abs_cos_theta(lwi));

        const float G =
            microfacet::smith_gtr2(local_angle::tan_theta(lwi), alpha_) *
            microfacet::smith_gtr2(local_angle::tan_theta(lwo), alpha_);

        return color_ * (
            D * F * G / (4 * local_angle::cos_theta(lwi)
                           * local_angle::cos_theta(lwo)));
    }

    SampleResult sample(
        const FVec3   &lwo,
        TransMode      mode,
        const Sample2 &sam) const noexcept override
    {
        if(lwo.z < EPS())
            return {};

        const FVec3 lwh = microfacet::sample_gtr2(alpha_, sam);
        if(lwh.z < EPS())
            return {};

        const FVec3 lwi = (2 * dot(lwo, lwh) * lwh - lwo).normalize();
        if(lwi.z < EPS())
            return {};

        const float D = microfacet::gtr2(local_angle::cos_theta(lwh), alpha_);

        const float F = refl_aux::dielectric_fresnel(
            eta_, 1, local_angle::abs_cos_theta(lwi));

        const float G =
            microfacet::smith_gtr2(local_angle::tan_theta(lwi), alpha_) *
            microfacet::smith_gtr2(local_angle::tan_theta(lwo), alpha_);

        const FSpectrum f = color_ * (
            D * F * G / (4 * local_angle::cos_theta(lwi)
                           * local_angle::cos_theta(lwo)));

        SampleResult ret;
        ret.lwi = lwi;
        ret.f   = f;
        ret.pdf = local_angle::cos_theta(lwh) * D / (4 * dot(lwo, lwh));

        return ret;
    }

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override
    {
        if(lwi.z < EPS() || lwo.z < EPS())
            return 0;

        const FVec3 lwh = (lwi + lwo).normalize();
        const float D = microfacet::gtr2(local_angle::cos_theta(lwh), alpha_);

        return local_angle::cos_theta(lwh) * D / (4 * dot(lwo, lwh));
    }
};

class SingleScatteredReflection : public BSDFComponent
{
    FSpectrum color_;

    const RhoDtTable *rhoDt_;
    real gf_, wf_, gb_, wb_;
    real alpha_, tau_d_;

public:

    SingleScatteredReflection(
        const FSpectrum &color,
        const RhoDtTable *rhoDt,
        real gf, real wf, real gb, real wb,
        real alpha, real tau_d) noexcept
        : BSDFComponent(BSDF_DIFFUSE)
    {
        color_ = color;
        rhoDt_ = rhoDt;
        gf_    = gf;
        wf_    = wf;
        gb_    = gb;
        wb_    = wb;
        alpha_ = alpha;
        tau_d_ = tau_d;
    }

    FSpectrum eval(
        const FVec3 &lwi,
        const FVec3 &lwo,
        TransMode    mode) const noexcept override
    {
        if(lwi.z < EPS() || lwo.z < EPS())
            return {};

        const real atti = rhoDt_->get_value(lwi);
        const real atto = rhoDt_->get_value(lwo);
        const real p = P(dot(lwi, lwo), gf_, gb_, wf_, wb_);
        const real e = std::exp(-tau_d_ / lwi.z - tau_d_ / lwo.z);
        const real dem = lwi.z + lwo.z;

        return color_ * (
            atti * atto * (alpha_ * p * (1 - e)) / dem);
    }

    SampleResult sample(
        const FVec3   &lwo,
        TransMode      mode,
        const Sample2 &sam) const noexcept override
    {
        if(lwo.z < EPS())
            return {};

        auto [lwi, pdf] = math::distribution::zweighted_on_hemisphere(
                                                    sam.u, sam.v);

        const FSpectrum f = eval(lwi, lwo, mode);

        SampleResult ret;
        ret.pdf = pdf;
        ret.f   = f;
        ret.lwi = lwi;

        return ret;
    }

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override
    {
        if(lwi.z < EPS() || lwo.z < EPS())
            return 0;
        return math::distribution::zweighted_on_hemisphere_pdf(lwi.z);
    }
};

class SingleScatteredTransmission : public BSDFComponent
{
    FSpectrum color_;

    const RhoDtTable *frontRhoDt_;
    const RhoDtTable *backRhoDt_;

    real gf_, wf_, gb_, wb_;
    real alpha_, tau_d_;

public:

    SingleScatteredTransmission(
        const FSpectrum &color,
        const RhoDtTable *frontRhoDt,
        const RhoDtTable *backRhoDt,
        real gf, real wf, real gb, real wb,
        real alpha, real tau_d) noexcept
        : BSDFComponent(BSDF_DIFFUSE)
    {
        color_ = color;
        frontRhoDt_ = frontRhoDt;
        backRhoDt_  = backRhoDt;
        gf_ = gf;
        wf_ = wf;
        gb_ = gb;
        wb_ = wb;
        alpha_ = alpha;
        tau_d_ = tau_d;
    }

    FSpectrum eval(
        const FVec3 &lwi,
        const FVec3 &lwo,
        TransMode    mode) const noexcept override
    {
        if(lwi.z * lwo.z > 0)
            return {};

        const real atti = (lwi.z > 0 ? frontRhoDt_ : backRhoDt_)->get_value(lwi);
        const real atto = (lwo.z > 0 ? frontRhoDt_ : backRhoDt_)->get_value(lwo);

        const real p = P(dot(lwi, lwo), gf_, gb_, wf_, wb_);

        if(std::abs(lwi.z + lwo.z) < EPS())
        {
            const real e = std::exp(-tau_d_ / lwo.z);
            return color_ * std::abs(atti * atto * alpha_ * tau_d_ * p * e
                                                 / (lwi.z * lwo.z));
        }

        const real e = std::exp(-tau_d_ / std::abs(lwi.z))
                     - std::exp(-tau_d_ / std::abs(lwo.z));

        const real val = std::abs(
            atti * atto * alpha_ * p * e
                        / (std::abs(lwi.z) - std::abs(lwo.z)));

        return color_ * val;
    }

    SampleResult sample(
        const FVec3   &lwo,
        TransMode      mode,
        const Sample2 &sam) const noexcept override
    {
        auto [lwi, pdf] = math::distribution::zweighted_on_hemisphere(
                                                    sam.u, sam.v);
        if(lwo.z > 0)
            lwi.z = -lwi.z;

        const FSpectrum f = eval(lwi, lwo, mode);

        SampleResult ret;
        ret.pdf = pdf;
        ret.f   = f;
        ret.lwi = lwi;

        return ret;
    }

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override
    {
        if(lwo.z * lwi.z >= 0)
            return 0;
        return math::distribution::zweighted_on_hemisphere_pdf(std::abs(lwi.z));
    }
};

class MultiScatteringReflection : public BSDFComponent
{
    FSpectrum color_;
    const RhoDtTable *rhoDt_;
    FSpectrum Rd_;

public:

    MultiScatteringReflection(
        const FSpectrum &color,
        const RhoDtTable *rhoDt, const FSpectrum &Rd) noexcept
        : BSDFComponent(BSDF_DIFFUSE)
    {
        color_ = color;
        rhoDt_ = rhoDt;
        Rd_ = Rd;
    }

    FSpectrum eval(
        const FVec3 &lwi,
        const FVec3 &lwo,
        TransMode    mode) const noexcept override
    {
        if(lwi.z < EPS() || lwo.z < EPS())
            return {};

        const real atti = rhoDt_->get_value(lwi);
        const real atto = rhoDt_->get_value(lwo);

        return color_ * atti * (Rd_ / PI_r) * atto;
    }

    SampleResult sample(
        const FVec3 &lwo,
        TransMode      mode,
        const Sample2 &sam) const noexcept override
    {
        if(lwo.z < EPS())
            return {};

        auto [lwi, pdf] = math::distribution::zweighted_on_hemisphere(
            sam.u, sam.v);

        const FSpectrum f = eval(lwi, lwo, mode);

        SampleResult ret;
        ret.pdf = pdf;
        ret.f = f;
        ret.lwi = lwi;

        return ret;
    }

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override
    {
        if(lwi.z < EPS() || lwo.z < EPS())
            return 0;
        return math::distribution::zweighted_on_hemisphere_pdf(lwi.z);
    }
};

class MultiScatteringTransmission : public BSDFComponent
{
    FSpectrum color_;
    const RhoDtTable *frontRhoDt_;
    const RhoDtTable *backRhoDt_;
    FSpectrum Td_;

public:

    MultiScatteringTransmission(
        const FSpectrum &color,
        const RhoDtTable *frontRhoDt, const RhoDtTable *backRhoDt,
        const FSpectrum &Td) noexcept
        : BSDFComponent(BSDF_DIFFUSE)
    {
        color_ = color;
        frontRhoDt_ = frontRhoDt;
        backRhoDt_  = backRhoDt;
        Td_         = Td;
    }

    FSpectrum eval(
        const FVec3 &lwi,
        const FVec3 &lwo,
        TransMode mode) const noexcept override
    {
        if(lwi.z * lwo.z >= 0)
            return {};

        const real atti = (lwi.z > 0 ? frontRhoDt_ : backRhoDt_)->get_value(lwi);
        const real atto = (lwo.z > 0 ? frontRhoDt_ : backRhoDt_)->get_value(lwo);

        return color_ * atti * (Td_ / PI_r) * atto;
    }
    
    SampleResult sample(
        const FVec3   &lwo,
        TransMode      mode,
        const Sample2 &sam) const noexcept override
    {
        auto [lwi, pdf] = math::distribution::zweighted_on_hemisphere(
                                                    sam.u, sam.v);
        if(lwo.z > 0)
            lwi.z = -lwi.z;

        const FSpectrum f = eval(lwi, lwo, mode);

        SampleResult ret;
        ret.pdf = pdf;
        ret.f   = f;
        ret.lwi = lwi;

        return ret;
    }

    real pdf(const FVec3 &lwi, const FVec3 &lwo) const noexcept override
    {
        if(lwo.z * lwi.z >= 0)
            return 0;
        return math::distribution::zweighted_on_hemisphere_pdf(std::abs(lwi.z));
    }
};

std::pair<FSpectrum, FSpectrum> computeRdAndTd(
    int n, const FSpectrum &color,
    real sigma_s, real sigma_a, real d,
    real gf, real gb, real wf, real wb) noexcept
{
    const real sigma_s_bar = (1 - (wb * gb + wf * gf)) * sigma_s;
    const real sigma_t_bar = sigma_a + sigma_s_bar;
    const real alpha_bar = sigma_s_bar / sigma_t_bar;
    const real sigma_tr = std::sqrt(3 * sigma_a * sigma_t_bar);

    auto sign = [](float x) { return x > 0 ? real(1) : real(-1); };

    const real l = 1 / sigma_t_bar;

    const FSpectrum A = (FSpectrum(1) + color)
                      / (FSpectrum(real(1.001)) - color);

    const real D = 1 / (3 * sigma_t_bar);

    const FSpectrum zb = 2 * A * D;

    auto z_r_i = [&](int i) { return real(2) * i * (d + zb + zb) + l; };
    auto z_v_i = [&](int i) { return real(2) * i * (d + zb + zb) - l - 2 * zb; };

    FSpectrum Rd, Td;
    for(int i = -n; i <= n; ++i)
    {
        const FSpectrum zri = z_r_i(i);
        const FSpectrum zvi = z_v_i(i);

        for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
        {
            Rd[c] +=
                sign(zri[c]) * std::exp(-sigma_tr * std::abs(zri[c]))
              - sign(zvi[c]) * std::exp(-sigma_tr * std::abs(zvi[c]));

            Td[c] +=
                sign(d - zri[c]) * std::exp(-sigma_tr * std::abs(d - zri[c]))
              - sign(d - zvi[c]) * std::exp(-sigma_tr * std::abs(d - zvi[c]));
        }
    }
    Rd *= alpha_bar / 2;
    Td *= alpha_bar / 2;

    return { Rd, Td };
}

class PaperMaterial : public Material
{
    Box<const NormalMapper> normal_mapper_;
    RC<const Texture2D> color_;

    Box<RhoDtTable> frontRhoDt_;
    Box<RhoDtTable> backRhoDt_;

    real gf_, gb_, wf_, wb_;
    real alpha_;
    real tau_d_;
    real front_eta_;
    real back_eta_;
    real front_roughness_;
    real back_roughness_;

    real sigma_s_, sigma_a_;
    real d_;

public:

    PaperMaterial(
        RC<const Texture2D> color,
        real gf, real gb, real wf, real wb,
        real front_eta, real back_eta, real d, real sigma_s, real sigma_a,
        real front_roughness, real back_roughness,
        Box<const NormalMapper> normal_mapper)
    {
        normal_mapper_ = std::move(normal_mapper);
        color_ = std::move(color);

        gf_  = gf;
        gb_  = gb;
        wf_  = wf;
        wb_  = wb;
        front_eta_ = front_eta;
        back_eta_  = back_eta;
        front_roughness_ = front_roughness;
        back_roughness_  = back_roughness;

        sigma_s_ = sigma_s;
        sigma_a_ = sigma_a;
        d_ = d;

        const real sigma_t = sigma_s + sigma_a;

        alpha_ = sigma_s / sigma_t;
        tau_d_ = sigma_t * d;

        frontRhoDt_ = newBox<RhoDtTable>(front_eta, front_roughness_);
        backRhoDt_  = newBox<RhoDtTable>(back_eta, back_roughness_);
    }

    ShadingPoint shade(
        const EntityIntersection &inct, Arena &arena) const override
    {
        const Coord shading_coord = normal_mapper_->reorient(
            inct.uv, inct.user_coord);

        const FSpectrum color = color_->sample_spectrum(inct.uv);

        if(inct.geometry_coord.in_positive_z_hemisphere(inct.wr))
        {
            // front side

            auto bsdf = arena.create<AggregateBSDF<5>>(
                inct.geometry_coord, shading_coord, color);

            auto ggx_refl = arena.create<GGXReflection>(
                color, front_roughness_, front_eta_);

            auto single_refl = arena.create<SingleScatteredReflection>(
                color, frontRhoDt_.get(), gf_, wf_, gb_, wb_, alpha_, tau_d_);

            auto single_refr = arena.create<SingleScatteredTransmission>(
                color, frontRhoDt_.get(), backRhoDt_.get(),
                gf_, wf_, gb_, wb_, alpha_, tau_d_);

            const auto [Rd, Td] = computeRdAndTd(
                1, color, sigma_s_, sigma_a_, d_, gf_, gb_, wf_, wb_);

            auto multi_refl = arena.create<MultiScatteringReflection>(
                color, frontRhoDt_.get(), Rd);

            auto multi_refr = arena.create<MultiScatteringTransmission>(
                color, frontRhoDt_.get(), backRhoDt_.get(), Td);

            bsdf->add_component(1, ggx_refl);
            bsdf->add_component(1, single_refl);
            bsdf->add_component(1, single_refr);
            bsdf->add_component(1, multi_refl);
            bsdf->add_component(1, multi_refr);

            ShadingPoint shd;
            shd.bsdf           = bsdf;
            shd.shading_normal = shading_coord.z;
            return shd;
        }

        // back side

        auto bsdf = arena.create<AggregateBSDF<5>>(
            -inct.geometry_coord, -shading_coord, color);

        auto ggx_refl = arena.create<GGXReflection>(
            color, back_roughness_, back_eta_);

        auto single_refl = arena.create<SingleScatteredReflection>(
            color, backRhoDt_.get(), gf_, wf_, gb_, wb_, alpha_, tau_d_);

        auto single_refr = arena.create<SingleScatteredTransmission>(
            color, backRhoDt_.get(), frontRhoDt_.get(),
            gf_, wf_, gb_, wb_, alpha_, tau_d_);

        const auto [Rd, Td] = computeRdAndTd(
            1, color, sigma_s_, sigma_a_, d_, gf_, gb_, wf_, wb_);

        auto multi_refl = arena.create<MultiScatteringReflection>(
            color, backRhoDt_.get(), Rd);

        auto multi_refr = arena.create<MultiScatteringTransmission>(
            color, backRhoDt_.get(), frontRhoDt_.get(), Td);

        bsdf->add_component(1, ggx_refl);
        bsdf->add_component(1, single_refl);
        bsdf->add_component(1, single_refr);
        bsdf->add_component(1, multi_refl);
        bsdf->add_component(1, multi_refr);

        ShadingPoint shd;
        shd.bsdf           = bsdf;
        shd.shading_normal = -shading_coord.z;
        return shd;
    }
};

} // namespace jensen_paper

RC<Material> create_paper(
    RC<const Texture2D> color,
    real gf, real gb, real wf, real wb,
    real front_eta, real back_eta,
    real d, real sigma_s, real sigma_a,
    real front_roughness, real back_roughness,
    Box<const NormalMapper> normal_mapper)
{
    return newRC<jensen_paper::PaperMaterial>(
        std::move(color), gf, gb, wf, wb,
        front_eta, back_eta, d, sigma_s, sigma_a,
        math::clamp(front_roughness, real(0.01), real(1)),
        math::clamp(back_roughness, real(0.01), real(1)),
        std::move(normal_mapper));
}

AGZ_TRACER_END
