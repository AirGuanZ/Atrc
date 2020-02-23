#pragma once

#include <agz/tracer/render/common.h>
#include <agz/tracer/core/render_target.h>

AGZ_TRACER_RENDER_BEGIN

struct ParticleTraceParams
{
    int min_depth = 5;
    int max_depth = 10;
    real cont_prob = real(0.9);

    Vec2 film_res = { 1, 1 };
};

void trace_particle(
    const ParticleTraceParams &params, const Scene &scene, Sampler &sampler,
    FilmFilterApplier::FilmGridView<Spectrum> &film, Arena &arena);

AGZ_TRACER_RENDER_END
