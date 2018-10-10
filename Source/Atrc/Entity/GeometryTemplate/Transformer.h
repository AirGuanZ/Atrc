#pragma once

#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

template<typename Ent, std::enable_if_t<std::is_base_of_v<Entity, Ent>, int> = 0>
class Transformer
    : public Ent
{
    Transform local2World_;
    Real areaSquareFactor_;

public:

    template<typename...Args>
    explicit Transformer(const Transform &local2World, Args&&...args)
        : Ent(std::forward<Args>(args)...), local2World_(local2World)
    {
        areaSquareFactor_ = 
            local2World_.ApplyToVector(Vec3r::UNIT_X()).LengthSquare() +
            local2World_.ApplyToVector(Vec3r::UNIT_Y()).LengthSquare() +
            local2World_.ApplyToVector(Vec3r::UNIT_Z()).LengthSquare();
    }

    bool HasIntersection(const Ray &r) const override
    {
        return Ent::HasIntersection(local2World_.ApplyInverseToRay(r));
    }

    bool EvalIntersection(const Ray &r, Intersection *inct) const
    {
        if(Ent::EvalIntersection(local2World_.ApplyInverseToRay(r), inct))
        {
            *inct = local2World_.ApplyToIntersection(*inct);
            return true;
        }
        return false;
    }

    AABB GetBoundingBox() const override
    {
        return local2World_.ApplyToAABB(Ent::GetBoundingBox());
    }

    Real SurfaceArea() const override
    {
        return areaSquareFactor_ * Ent::SurfaceArea();
    }
};

AGZ_NS_END(Atrc)
