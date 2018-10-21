#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Material/BlackMaterial.h>

AGZ_NS_BEG(Atrc)

template<typename L, std::enable_if_t<std::is_base_of_v<GeometricLight, L>, int> = 0>
class GeometricLightEntity : public Entity
{
    L light_;

    AGZ_FORCEINLINE const Geometry *Geo() const
    {
        return light_.GetGeometry();
    }

public:

    template<typename...Args>
    explicit GeometricLightEntity(Args&&...args)
        : light_(std::forward<Args>(args)...)
    {
        
    }

    bool HasIntersection(const Ray &r) const override
    {
        return Geo()->HasIntersection(r);
    }

    bool FindIntersection(const Ray &r, SurfacePoint *sp) const override
    {
        if(!Geo()->FindIntersection(r, sp))
            return false;
        sp->entity = this;
        return true;
    }

    AABB WorldBound() const override
    {
        return Geo()->WorldBound();
    }

    const Material *GetMaterial(const SurfacePoint &sp) const override
    {
        return &STATIC_BLACK_MATERIAL;
    }

    const Light *AsLight() const override
    {
        return &light_;
    }
};

AGZ_NS_END(Atrc)
