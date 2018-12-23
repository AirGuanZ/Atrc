#pragma once

#include <Atrc/Lib/Core/Entity.h>
#include <Atrc/Lib/Core/Light.h>

namespace Atrc
{

class GeometricDiffuseLight : public Entity, public Light
{
    const Geometry *geometry_;
    Spectrum radiance_;

public:

    GeometricDiffuseLight(const Geometry *geometry, const Spectrum &radiance) noexcept;

    bool HasIntersection(const Ray &r) const noexcept override;

    bool FindIntersection(const Ray &r, Intersection *inct) const noexcept override;

    AABB GetWorldBound() const override;

    const Material *GetMaterial(const Intersection &inct) const noexcept override;

    const Light *AsLight() const noexcept override;

    Light *AsLight() noexcept override;

    void PreprocessScene(const Scene &scene) override;

    SampleWiResult SampleWi(const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample) const noexcept override;

    Real SampleWiAreaPDF(const Vec3 &pos, const Vec3 &nor, const Intersection &inct, const ShadingPoint &shd) const noexcept override;

    Real SampleWiNonAreaPDF(const Vec3 &wi, const Intersection &inct, const ShadingPoint &shd) const noexcept override;

    SampleWiResult SampleWi(const Vec3 &medPos, const Vec3 &sample) const noexcept override;

    Real SampleWiAreaPDF(const Vec3 &pos, const Vec3 &nor, const Vec3 &medPos) const noexcept override;

    Real SampleWiNonAreaPDF(const Vec3 &wi, const Vec3 &medPos) const noexcept override;

    //Real SampleWiPDF(const Vec3 &pos, const Vec3 &nor, const Intersection &inct, const ShadingPoint &shd, bool onLight) const noexcept override;

    Spectrum AreaLe(const Intersection &inct) const noexcept override;

    Spectrum NonAreaLe(const Ray &r) const noexcept override;
};

} // namespace Atrc
