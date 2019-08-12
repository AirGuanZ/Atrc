#pragma once

#include <agz/tracer/core/bsdf.h>

AGZ_TRACER_BEGIN

// IMPROVE: handle black fringes
template<int MAX_BSDF_CNT>
class BSDFAggregate : public LocalBSDF
{
    const BSDF *bsdfs_[MAX_BSDF_CNT];
    int bsdf_count_;

public:

    BSDFAggregate(const Coord &geometry_coord, const Coord &shading_coord) noexcept
        : LocalBSDF(geometry_coord, shading_coord),
          bsdfs_{ nullptr }, bsdf_count_(0)
    {
        
    }

    void add(const BSDF *bsdf)
    {
        assert(bsdf_count_ < MAX_BSDF_CNT);
        bsdfs_[bsdf_count_++] = bsdf;
    }

    Spectrum eval(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode transport_mode) const noexcept override
    {
        assert(bsdf_count_ > 0);

        Vec3 local_in  = shading_coord_.global_to_local(in_dir).normalize();
        Vec3 local_out = shading_coord_.global_to_local(out_dir).normalize();

        Spectrum ret;
        for(int i = 0; i < bsdf_count_; ++i)
            ret += bsdfs_[i]->eval(local_in, local_out, transport_mode);
        return ret;
    }

    real proj_wi_factor(const Vec3 &wi) const noexcept override
    {
        Vec3 lwi = shading_coord_.global_to_local(wi).normalize();

        real sum = 0;
        for(int i = 0; i < bsdf_count_; ++i)
            sum += bsdfs_[i]->proj_wi_factor(lwi);
        return sum / bsdf_count_;
    }

    BSDFSampleResult sample(const Vec3 &out_dir, TransportMode transport_mode, const Sample3 &sam) const noexcept override
    {
        assert(bsdf_count_ > 0);

        auto [bsdf_idx, new_sam_u] = math::distribution::extract_uniform_int(sam.u, 0, bsdf_count_);
        auto bsdf = bsdfs_[bsdf_idx];

        Vec3 local_out = shading_coord_.global_to_local(out_dir).normalize();
        auto ret = bsdf->sample(local_out, transport_mode, { new_sam_u, sam.v, sam.w });

        if(!ret.dir)
            return BSDF_SAMPLE_RESULT_INVALID;

        ret.dir = ret.dir.normalize();

        if(ret.is_delta)
        {
            ret.pdf /= bsdf_count_;
            ret.dir = shading_coord_.local_to_global(ret.dir).normalize();
            return ret;
        }

        for(int i = 0; i < bsdf_count_; ++i)
        {
            if(i == bsdf_idx)
                continue;
            ret.f   += bsdfs_[i]->eval(ret.dir, local_out, transport_mode);
            ret.pdf += bsdfs_[i]->pdf(ret.dir, local_out, transport_mode);
        }
        ret.pdf /= bsdf_count_;
        ret.dir = shading_coord_.local_to_global(ret.dir).normalize();

        return ret;
    }

    real pdf(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode transport_mode) const noexcept override
    {
        assert(bsdf_count_ > 0);

        Vec3 local_in  = shading_coord_.global_to_local(in_dir).normalize();
        Vec3 local_out = shading_coord_.global_to_local(out_dir).normalize();

        real ret = 0;
        for(int i = 0; i < bsdf_count_; ++i)
            ret += bsdfs_[i]->pdf(local_in, local_out, transport_mode);
        return ret / bsdf_count_;
    }

    bool is_delta() const noexcept override
    {
        for(int i = 0; i < bsdf_count_; ++i)
        {
            if(!bsdfs_[i]->is_delta())
                return false;
        }
        return true;
    }

    Spectrum albedo() const noexcept override
    {
        Spectrum ret;
        for(int i = 0; i < bsdf_count_; ++i)
            ret += bsdfs_[i]->albedo();
        return ret;
    }
};

AGZ_TRACER_END
