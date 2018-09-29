#pragma once

#include <Atrc/Common.h>
#include <Atrc/Math/Ray.h>

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Entity
{
public:

    virtual ~Entity() = default;

    virtual bool HasIntersection(const Ray &r) const;

    virtual Option<Intersection> EvalIntersection(const Ray &r) const = 0;

    virtual RC<BxDF> GetBxDF(const Intersection &inct) const = 0;

    virtual Spectrum AmbientRadiance() const;
};

AGZ_NS_END(Atrc)
