#include <agz/tracer/render/pssmlt.h>

AGZ_TRACER_RENDER_BEGIN

namespace pssmlt
{

PSSMLTSampler::PSSMLTSampler(
    real sigma, real large_mut_prob,
    const NativeSampler &native_sampler)
    : uniform_sampler_(native_sampler),
      sigma_(sigma),
      large_mut_prob_(large_mut_prob), is_curr_large_(true),
      curr_iter_(0), last_large_iter_(0),
      next_dim_(0)
{

}

Sample1 PSSMLTSampler::sample1()
{
    const size_t dim = next_dim_++;
    if(dim >= primary_samples_.size())
        primary_samples_.resize(dim + 1);

    auto &s = primary_samples_[dim];

    if(s.dirty_iter < last_large_iter_)
    {
        s.value = uniform_sampler_.sample1().u;
        s.dirty_iter = last_large_iter_;
    }

    s.backup_value = s.value;
    s.backup_dirty_iter = s.dirty_iter;

    if(is_curr_large_)
        s.value = uniform_sampler_.sample1().u;
    else
    {
        const uint64_t small_iters = curr_iter_ - s.dirty_iter;
        const real normal_dis = normal_dis_(uniform_sampler_.rng());
        const real eff_sigma = sigma_ * std::sqrt(real(small_iters));

        s.value += normal_dis * eff_sigma;
        s.value -= std::floor(s.value);
    }

    s.dirty_iter = curr_iter_;

    return { s.value };
}

Sample2 PSSMLTSampler::sample2()
{
    const real u = sample1().u;
    const real v = sample1().u;
    return { u, v };
}

Sample3 PSSMLTSampler::sample3()
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    return { u, v, w };
}

Sample4 PSSMLTSampler::sample4()
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    const real r = sample1().u;
    return { u, v, w, r };
}

Sample5 PSSMLTSampler::sample5()
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    const real r = sample1().u;
    const real s = sample1().u;
    return { u, v, w, r, s };
}

void PSSMLTSampler::new_iteration()
{
    curr_iter_++;
    next_dim_ = 0;
    is_curr_large_ = uniform_sampler_.sample1().u < large_mut_prob_;
}

void PSSMLTSampler::accept()
{
    if(is_curr_large_)
        last_large_iter_ = curr_iter_;
}

void PSSMLTSampler::reject()
{
    for(auto &s : primary_samples_)
    {
        if(s.dirty_iter == curr_iter_)
        {
            s.value = s.backup_value;
            s.dirty_iter = s.backup_dirty_iter;
        }
    }
    --curr_iter_;
}

} // namespace pssmlt

AGZ_TRACER_RENDER_END
