#include <agz/tracer/core/medium.h>
#include <agz/tracer/utility/phase_function.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class HomogeneousMedium : public Medium
{
    Spectrum sigma_a_; // 吸收系数
    Spectrum sigma_s_; // 散射系数
    Spectrum sigma_t_; // 衰减系数
    real g_ = 0;       // 散射不对称因子

    Spectrum albedo() const
    {
        return !sigma_t_ ? Spectrum(1) : sigma_s_ / sigma_t_;
    }

public:

    void initialize(const Spectrum &sigma_a, const Spectrum &sigma_s, real g)
    {
        AGZ_HIERARCHY_TRY

        sigma_a_ = sigma_a;
        sigma_s_ = sigma_s;
        sigma_t_ = sigma_a + sigma_s;

        g_ = g;
        if(g_ <= -1 || g_ >= 1)
            throw ObjectConstructionException("invalid g value: " + std::to_string(g_));

        AGZ_HIERARCHY_WRAP("in initializing homogeneous medium")
    }

    Spectrum tr(const Vec3 &a, const Vec3 &b) const noexcept override
    {
        Spectrum exp = -sigma_t_ * (a - b).length();
        return {
            std::exp(exp.r),
            std::exp(exp.g),
            std::exp(exp.b)
        };
    }

    SampleOutScatteringResult sample_scattering(const Vec3 &a, const Vec3 &b, Sampler &sampler) const noexcept override
    {
        Sample1 sam = sampler.sample1();
        if(!sigma_s_)
            return { std::nullopt, Spectrum(1) };

        real t_max = (b - a).length();
        auto[color_channel, new_sam] = math::distribution::extract_uniform_int(sam.u, 0, SPECTRUM_COMPONENT_COUNT);
        real st = -std::log(new_sam) / sigma_t_[color_channel];

        bool sample_medium = st < t_max;
        Spectrum tr;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            tr[i] = std::exp(-sigma_t_[i] * (std::min)(st, t_max));
        Spectrum density = sample_medium ? sigma_s_ * tr : tr;

        real pdf = 0;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            pdf += density[i];
        pdf /= SPECTRUM_COMPONENT_COUNT;
        pdf = (std::max)(pdf, EPS);

        SampleOutScatteringResult result;

        if(sample_medium)
        {
            MediumScattering inct;
            inct.pos    = lerp(a, b, st);
            inct.medium = this;
            inct.t      = st;
            inct.wr     = (a - b) / t_max;
            result.scattering_point = inct;
        }
        result.throughput = tr / pdf;

        return result;
    }

    //SampleMediumResult sample(const Vec3 &o, const Vec3 &d, real t_min, real t_max, Sampler &sampler) const noexcept override
    //{
    //    Sample1 sam = sampler.sample1();

    //    // 这里scale一下t_min/t_max，就好像d的长度是1一样，最后算位置的时候再scale回来就行了
    //    real len = d.length();
    //    t_min *= len;
    //    t_max *= len;

    //    if(!sigma_s_)
    //        return SAMPLE_MEDIUM_RESULT_INVALID;

    //    real t_interval = t_max - t_min;
    //    assert(t_interval > 0);

    //    auto [channel, new_sam] = math::distribution::extract_uniform_int(sam.u, 0, SPECTRUM_COMPONENT_COUNT);
    //    real st = -std::log(new_sam) / sigma_t_[channel];

    //    bool sample_medium = st < t_interval;
    //    Spectrum tr;
    //    for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
    //        tr[i] = std::exp(-sigma_t_[i] * (std::min)(st, t_interval));
    //    Spectrum density = sample_medium ? sigma_s_ * tr : tr;

    //    real pdf = 0;
    //    for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
    //        pdf += density[i];
    //    pdf /= SPECTRUM_COMPONENT_COUNT;
    //    pdf = (std::max)(pdf, EPS);

    //    SampleMediumResult ret;

    //    if(!sample_medium)
    //    {
    //        ret.pdf = pdf;
    //        return ret;
    //    }

    //    ret.inct.medium = this;
    //    ret.inct.t      = (st + t_min) / len;
    //    ret.inct.wr     = -d;
    //    ret.pdf         = pdf;
    //    ret.inct.pos    = o + d * ret.inct.t;

    //    return ret;
    //}

    ShadingPoint shade(const MediumScattering &inct, Arena &arena) const noexcept override
    {
        assert(!inct.invalid());
        auto bsdf = arena.create<HenyeyGreensteinPhaseFunction>(g_, sigma_s_, albedo());
        return { bsdf };
    }
};

std::shared_ptr<Medium> create_homogeneous_medium(
    const Spectrum &sigma_a,
    const Spectrum &sigma_s,
    real g)
{
    auto ret = std::make_shared<HomogeneousMedium>();
    ret->initialize(sigma_a, sigma_s, g);
    return ret;
}

AGZ_TRACER_END
