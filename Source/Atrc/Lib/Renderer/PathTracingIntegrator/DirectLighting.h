#pragma once

#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Core/SurfacePoint.h>

namespace Atrc
{
    
std::pair<Spectrum, Option<BSDF::SampleWiResult>> ComputeDirectLighting(
    const Scene &scene, const Intersection &inct, const ShadingPoint &shd,
    bool sampleAllLights, bool useMIS, bool considerMedium, Sampler *sampler);

} // namespace Atrc
