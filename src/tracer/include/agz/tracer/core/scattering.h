#pragma once

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/utility/math.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

/**
 * @brief 空间中的散射点
 */
class ScatteringPoint
{
    misc::variant_t<EntityIntersection, MediumIntersection> inct_;
    const BSDF *bsdf_ = nullptr;

public:

    ScatteringPoint() = default;

    ScatteringPoint(const EntityIntersection &inct, Arena &arena)
    {
        auto shd = inct.material->shade(inct, arena);
        inct_ = inct;
        bsdf_ = shd.bsdf;
    }

    ScatteringPoint(const MediumIntersection &inct, Arena &arena)
    {
        auto shd = inct.medium->shade(inct, arena);
        inct_ = inct;
        bsdf_ = shd.bsdf;
    }

    ScatteringPoint(const ScatteringPoint&)            = default;
    ScatteringPoint &operator=(const ScatteringPoint&) = default;

    bool valid() const noexcept
    {
        return bsdf_ != nullptr;
    }

    Vec3 pos() const
    {
        return match_variant(inct_,
            [](const EntityIntersection &inct)
        {
            return inct.pos;
        },
            [](const MediumIntersection &inct)
        {
            return inct.pos;
        });
    }

    Vec3 wr() const
    {
        return match_variant(inct_,
            [](const EntityIntersection &inct)
        {
            return inct.wr;
        },
            [](const MediumIntersection &inct)
        {
            return inct.wr;
        });
    }

    Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept
    {
        return bsdf_->eval(wi, wo, mode);
    }

    real proj_wi_factor(const Vec3 &wi) const noexcept
    {
        return bsdf_->proj_wi_factor(wi);
    }

    BSDFSampleResult sample(const Vec3 &wo, const Sample3 &sam, TransportMode mode) const noexcept
    {
        return bsdf_->sample(wo, mode, sam);
    }

    real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept
    {
        return bsdf_->pdf(wi, wo, mode);
    }

    bool is_on_surface() const noexcept
    {
        return inct_.is<EntityIntersection>();
    }

    const EntityIntersection &as_entity_inct() const
    {
        return inct_.as<EntityIntersection>();
    }

    const BSDF *bsdf() const noexcept
    {
        return bsdf_;
    }

    const Medium *medium(const Vec3 &wr) const
    {
        return match_variant(inct_,
            [&](const EntityIntersection &inct)
        {
            return dot(wr, inct.geometry_coord.z) > 0 ? inct.medium_out : inct.medium_in;
        },
            [](const MediumIntersection &inct)
        {
            return inct.medium;
        });
    }

    real t() const
    {
        return match_variant(inct_,
            [](const EntityIntersection &inct)
        {
            return inct.t;
        },
            [](const MediumIntersection &inct)
        {
            return inct.t;
        });
    }
};

AGZ_TRACER_END
