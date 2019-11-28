#pragma once

#include <agz/tracer/core/intersection.h>

AGZ_TRACER_BEGIN

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam);

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light, const MediumScattering &scattering, const ShadingPoint &shd, const Sample5 &sam);

Spectrum mis_sample_nonarea_light(
    const Scene &scene, const NonareaLight *light, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam);

Spectrum mis_sample_nonarea_light(
    const Scene &scene, const NonareaLight *light, const MediumScattering &scattering, const ShadingPoint &shd, const Sample5 &sam);

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht, const EntityIntersection &inct, const ShadingPoint &shd, const Sample5 &sam);

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht, const MediumScattering &scattering, const ShadingPoint &shd, const Sample5 &sam);

/**
 * @brief 计算bsdf采样 + 光源采样中的前半部分
 *
 * 后三个参数分别用于输出bsdf采样结果以及采样射线与场景的求交情况
 */
Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, const Sample3 &sam,
    BSDFSampleResult &bsdf_sample, bool &has_ent_inct, EntityIntersection &ent_inct);

Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, const Sample3 &sam);

/**
 * @brief 计算bsdf采样 + 光源采样中的前半部分
 *
 * 后三个参数分别用于输出bsdf采样结果以及采样射线与场景的求交情况
 */
Spectrum mis_sample_bsdf(
    const Scene &scene, const MediumScattering &scattering, const ShadingPoint &shd, const Sample3 &sam,
    BSDFSampleResult &bsdf_sample, bool &has_ent_inct, EntityIntersection &ent_inct);

Spectrum mis_sample_bsdf(
    const Scene &scene, const MediumScattering &scattering, const ShadingPoint &shd, const Sample3 &sam);

AGZ_TRACER_END
