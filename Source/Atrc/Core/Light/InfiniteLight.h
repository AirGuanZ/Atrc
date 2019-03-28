#pragma once

#include <Atrc/Core/Core/Light.h>
#include <Atrc/Core/Core/Transform.h>

namespace Atrc
{
    
class InfiniteLight : public Light
{
    Vec3 worldCentre_;
    Real worldRadius_;

protected:

    Transform local2World_;

public:

    InfiniteLight(const Transform &local2World) noexcept;

    void PreprocessScene(const Scene &scene) override;

    SampleWiResult SampleWi(const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample) const noexcept override;

    Real SampleWiAreaPDF(const Vec3 &pos, const Vec3 &nor, const Intersection &inct, const ShadingPoint &shd) const noexcept override;

    Real SampleWiNonAreaPDF(const Vec3 &wi, const Intersection &inct, const ShadingPoint &shd) const noexcept override;

    SampleWiResult SampleWi(const Vec3 &pos, const Vec3 &sample) const noexcept override;

    Real SampleWiAreaPDF(const Vec3 &pos, const Vec3 &nor, const Vec3 &medPos) const noexcept override;

    Real SampleWiNonAreaPDF(const Vec3 &wi, const Vec3 &medPos) const noexcept override;

    Spectrum AreaLe(const Intersection &inct) const noexcept override;
};

} // namespace Atrc
