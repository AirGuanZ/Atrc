#pragma once

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

FSpectrum mis_sample_area_light(
    const Scene &scene,
    const AreaLight *light,
    const EntityIntersection &inct,
    const ShadingPoint &shd,
    Sampler &sampler);

FSpectrum mis_sample_area_light(
    const Scene &scene,
    const AreaLight *light,
    const MediumScattering &scattering,
    const BSDF *phase_function,
    Sampler &sampler);

FSpectrum mis_sample_envir_light(
    const Scene &scene,
    const EnvirLight *light,
    const EntityIntersection &inct,
    const ShadingPoint &shd,
    Sampler &sampler);

FSpectrum mis_sample_envir_light(
    const Scene &scene,
    const EnvirLight *light,
    const MediumScattering &scattering,
    const BSDF *phase_function,
    Sampler &sampler);

FSpectrum mis_sample_light(
    const Scene &scene,
    const Light *lht,
    const EntityIntersection &inct,
    const ShadingPoint &shd,
    Sampler &sampler);

FSpectrum mis_sample_light(
    const Scene &scene,
    const Light *lht,
    const MediumScattering &scattering,
    const BSDF *phase_function,
    Sampler &sampler);

/**
 * @brief compute BSDF sampling part in MIS direct illumination
 *
 * the last 3 parameters are used for receiving the BSDF sampling result
 */
FSpectrum mis_sample_bsdf(
    const Scene &scene,
    const EntityIntersection &inct,
    const ShadingPoint &shd,
    Sampler &sampler,
    BSDFSampleResult &bsdf_sample,
    bool &has_ent_inct,
    EntityIntersection &ent_inct);

FSpectrum mis_sample_bsdf(
    const Scene &scene,
    const EntityIntersection &inct,
    const ShadingPoint &shd,
    Sampler &sampler);

/**
 * @brief compute phase function sampling part in MIS direct illumination
 *
 * the last 3 parameters are receiver of the phase function sampling result
 */
FSpectrum mis_sample_bsdf(
    const Scene &scene,
    const MediumScattering &scattering,
    const BSDF *phase_function,
    Sampler &sampler,
    BSDFSampleResult &bsdf_sample,
    bool &has_ent_inct, EntityIntersection &ent_inct);

FSpectrum mis_sample_bsdf(
    const Scene &scene,
    const MediumScattering &scattering,
    const BSDF *phase_function,
    Sampler &sampler);

AGZ_TRACER_END
