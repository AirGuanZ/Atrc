#pragma once

#include <agz/tracer/core/sampler.h>
#include <agz/tracer/render/common.h>

AGZ_TRACER_RENDER_BEGIN

namespace pssmlt
{

class PSSMLTSampler : public Sampler
{
public:

    PSSMLTSampler(
        real sigma, real large_mut_prob,
        const NativeSampler &native_sampler);

    Sample1 sample1() override;
    Sample2 sample2() override;
    Sample3 sample3() override;
    Sample4 sample4() override;
    Sample5 sample5() override;

    void new_iteration();

    void accept();

    void reject();

private:

    struct PrimarySample
    {
        real value = 0;
        uint64_t dirty_iter = 0;

        real backup_value = 0;
        uint64_t backup_dirty_iter = 0;
    };

    NativeSampler uniform_sampler_;
    std::normal_distribution<real> normal_dis_;

    real sigma_;
    real large_mut_prob_;

    bool is_curr_large_;
    uint64_t curr_iter_;
    uint64_t last_large_iter_;

    size_t next_dim_;

    std::vector<PrimarySample> primary_samples_;
};

} // namespace pssmlt

AGZ_TRACER_RENDER_END
