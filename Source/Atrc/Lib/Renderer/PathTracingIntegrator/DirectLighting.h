#pragma once

#include <tuple>

#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Core/SurfacePoint.h>

namespace Atrc
{
    
std::tuple<Spectrum, std::optional<BSDF::SampleWiResult>, std::optional<Intersection>> ComputeDirectLighting(
    const Scene &scene, const Intersection &inct, const ShadingPoint &shd,
    bool sampleAllLights, bool considerMedium, Sampler *sampler);

std::tuple<Spectrum, PhaseFunction::SampleWiResult, std::optional<Intersection>> ComputeDirectLighting(
    const Scene &scene, const MediumPoint &mpnt, const MediumShadingPoint &mshd,
    bool sampleAllLights, Sampler *sampler);

} // namespace Atrc
