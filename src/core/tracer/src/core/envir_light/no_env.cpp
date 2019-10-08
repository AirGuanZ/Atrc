#include <agz/tracer/core/light.h>

AGZ_TRACER_BEGIN

class NoEnv : public EnvirLight
{
public:

    LightSampleResult sample(const Vec3 &ref, const Sample5 &sam) const noexcept override
    {
        return LIGHT_SAMPLE_RESULT_NULL;
    }

    real pdf(const Vec3 &ref, const Vec3 &) const noexcept override
    {
        return 0;
    }

    Spectrum power() const noexcept override
    {
        return {};
    }

    Spectrum radiance(const Vec3 &ref, const Vec3 &ref_to_light, Vec3 *light_pnt) const noexcept override
    {

        if(light_pnt)
            *light_pnt = ref + 4 * world_radius_ * ref_to_light.normalize();
        return {};
    }
};

std::shared_ptr<NonareaLight> create_no_env()
{
    return std::make_shared<NoEnv>();
}

AGZ_TRACER_END
