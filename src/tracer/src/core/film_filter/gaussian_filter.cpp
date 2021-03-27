#include <random>

#include <agz/tracer/core/film_filter.h>
#include <agz-utils/misc.h>

AGZ_TRACER_BEGIN

class GaussianFilter : public FilmFilter
{
    real radius_ = 0;
    real alpha_  = 0;
    real exp_    = 0;

    real norm_factor_ = 1;

    void init_norm_factor()
    {
        norm_factor_ = 1;

        std::default_random_engine rng(42);
        std::uniform_real_distribution<real> dis(-radius_, radius_);

        const real pdf = 1 / (4 * radius_ * radius_);

        real sum = 0;

        const int N = 100000;
        for(int i = 0; i < N; ++i)
        {
            const real x = dis(rng);
            const real y = dis(rng);
            const real f = eval(x, y);
            sum += f / pdf;
        }

        if(sum > 0)
            norm_factor_ = N / sum;
    }

public:

    GaussianFilter(real radius, real alpha)
    {
        if(radius <= 0)
            throw ObjectConstructionException(
                "invalid radius value: " + std::to_string(radius));
            
        if(alpha <= 0)
            throw ObjectConstructionException(
                "invalid alpha value: " + std::to_string(alpha));

        radius_ = radius;
        alpha_  = alpha;
        exp_    = std::exp(-alpha * radius * radius);

        init_norm_factor();
    }

    real radius() const noexcept override
    {
        return radius_;
    }

    real eval(real rel_x, real rel_y) const noexcept override
    {
        static auto gaussian = [](real d, real expv, real alpha)
        {
            return (std::max)(real(0), real(std::exp(-alpha * d * d) - expv));
        };
        return norm_factor_ * gaussian(rel_x, exp_, alpha_) * gaussian(rel_y, exp_, alpha_);
    }
};

RC<FilmFilter> create_gaussian_filter(
    real radius, real alpha)
{
    return newRC<GaussianFilter>(radius, alpha);
}

AGZ_TRACER_END
