﻿#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/scattering.h>

AGZ_TRACER_BEGIN

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam);

Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, const Sample3 &sam);

Spectrum mis_sample_area_light(const Scene &scene, const AreaLight *light, const ScatteringPoint &pnt, const Sample5 sam);

Spectrum mis_sample_envir_light(
    const Scene &scene, const EnvirLight *env, const EntityIntersection &inct, const ShadingPoint &shd, const Sample3 &sam);

Spectrum mis_sample_envir_light(const Scene &scene, const EnvirLight *env, const ScatteringPoint &pnt, const Sample3 sam);

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam);

Spectrum mis_sample_light(const Scene &scene, const Light *lht, const ScatteringPoint &pnt, const Sample5 sam);

Spectrum mis_sample_scattering(const Scene &scene, const ScatteringPoint &pnt, const Sample3 &sam);

AGZ_TRACER_END
