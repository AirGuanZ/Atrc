#pragma once

#include <agz/tracer/core/bssrdf.h>
#include <agz/tracer/core/medium.h>

AGZ_TRACER_BEGIN

class PathTracedBSSRDF : public BSSRDF
{
    const Medium *medium_;

public:

    PathTracedBSSRDF(const EntityIntersection &xo, const Medium *medium);

    BSSRDFSampleResult sample(const Vec3 &xo_wi, TransportMode mode, const Sample4 &sam, Arena &arena) const override;
};

AGZ_TRACER_END
