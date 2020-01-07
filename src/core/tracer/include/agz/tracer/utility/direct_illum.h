#pragma once

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/sampler.h>

AGZ_TRACER_BEGIN

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light, const EntityIntersection &inct, const ShadingPoint &shd, Sampler &sampler);

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light, const MediumScattering &scattering, const BSDF *phase_function, Sampler &sampler);

Spectrum mis_sample_nonarea_light(
    const Scene &scene, const EnvirLight *light, const EntityIntersection &inct, const ShadingPoint &shd, Sampler &sampler);

Spectrum mis_sample_nonarea_light(
    const Scene &scene, const EnvirLight *light, const MediumScattering &scattering, const BSDF *phase_function, Sampler &sampler);

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht, const EntityIntersection &inct, const ShadingPoint &shd, Sampler &sampler);

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht, const MediumScattering &scattering, const BSDF *phase_function, Sampler &sampler);

/**
 * @brief 计算bsdf采样 + 光源采样中的前半部分
 *
 * 后三个参数分别用于输出bsdf采样结果以及采样射线与场景的求交情况
 */
Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, Sampler &sampler,
    BSDFSampleResult &bsdf_sample, bool &has_ent_inct, EntityIntersection &ent_inct);

Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, Sampler &sampler);

/**
 * @brief 计算bsdf采样 + 光源采样中的前半部分
 *
 * 后三个参数分别用于输出bsdf采样结果以及采样射线与场景的求交情况
 */
Spectrum mis_sample_bsdf(
    const Scene &scene, const MediumScattering &scattering, const BSDF *phase_function, Sampler &sampler,
    BSDFSampleResult &bsdf_sample, bool &has_ent_inct, EntityIntersection &ent_inct);

Spectrum mis_sample_bsdf(
    const Scene &scene, const MediumScattering &scattering, const BSDF *phase_function, Sampler &sampler);

AGZ_TRACER_END
