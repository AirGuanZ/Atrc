#pragma once

#include <Atrc/Lib/Core/Light.h>
#include <Atrc/Lib/Core/Texture.h>

namespace Atrc
{

class CubeEnvironmentLight : public Light
{
    // 0 : +X
    // 1 : +Y
    // 2 : +Z
    // 3 : -X
    // 4 : -Y
    // 5 : -Z
    const Texture *envTex_[6];

    Vec3 worldCentre_;
    Real worldRadius_;

public:

    explicit CubeEnvironmentLight(const Texture **envTex) noexcept;

    void PreprocessScene(const Scene &scene) override;

    SampleWiResult SampleWi(const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const Vec3 &pos, const Intersection &inct, const ShadingPoint &shd, bool onLight) const noexcept override;

    Spectrum AreaLe(const Intersection &inct) const noexcept override;

    Spectrum NonAreaLe(const Ray &r) const noexcept override;
};

} // namespace Atrc
