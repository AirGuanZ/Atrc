#pragma once

#include <agz/tracer/core/bsdf.h>

AGZ_TRACER_BEGIN

class BSDFAdder : public BSDF
{
    std::vector<const BSDF*> bsdfs_;

public:

    BSDFAdder() noexcept
    {
        bsdfs_.reserve(4);
    }

    void add(const BSDF *bsdf)
    {
        bsdfs_.push_back(bsdf);
    }

    Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
    {
        Spectrum ret;
        for(size_t i = 0; i < bsdfs_.size(); ++i)
            ret += bsdfs_[i]->eval(wi, wo, mode);
        return ret;
    }

    BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
    {
        assert(!bsdfs_.empty());

        auto [bsdf_idx, new_sam_u] = math::distribution::extract_uniform_int(sam.u, 0, int(bsdfs_.size()));
        const BSDF *bsdf = bsdfs_[bsdf_idx];

        auto ret = bsdf->sample(wo, mode, { new_sam_u, sam.v, sam.w });

        if(ret.invalid())
            return BSDF_SAMPLE_RESULT_INVALID;

        ret.dir = ret.dir.normalize();

        if(ret.is_delta)
        {
            ret.pdf /= bsdfs_.size();
            return ret;
        }

        for(size_t i = 0; i < bsdfs_.size(); ++i)
        {
            if(i == bsdf_idx)
                continue;
            ret.f   += bsdfs_[i]->eval(ret.dir, wo, mode);
            ret.pdf += bsdfs_[i]->pdf(ret.dir, wo, mode);
        }
        ret.pdf /= bsdfs_.size();

        return ret;
    }

    real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
    {
        assert(!bsdfs_.empty());

        real ret = 0;
        for(size_t i = 0; i < bsdfs_.size(); ++i)
            ret += bsdfs_[i]->pdf(wi, wo, mode);
        return ret / bsdfs_.size();
    }

    bool is_delta() const noexcept override
    {
        for(size_t i = 0; i < bsdfs_.size(); ++i)
        {
            if(!bsdfs_[i]->is_delta())
                return false;
        }
        return true;
    }

    Spectrum albedo() const noexcept override
    {
        Spectrum ret;
        for(size_t i = 0; i < bsdfs_.size(); ++i)
            ret += bsdfs_[i]->albedo();
        return ret;
    }

    bool is_black() const noexcept override
    {
        for(size_t i = 0; i < bsdfs_.size(); ++i)
        {
            if(!bsdfs_[i]->is_black())
                return false;
        }
        return true;
    }
};

AGZ_TRACER_END
