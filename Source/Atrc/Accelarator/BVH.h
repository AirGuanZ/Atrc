#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class BVH : public Entity
{
public:

    struct Node
    {
        bool isLeaf;
        union
        {
            struct
            {
                const AABB *bound;
                uint32_t rightChild;
            } internal;

            struct
            {
                uint32_t start;
                uint32_t end;
            } leaf;
        };
    };

    using ConstEntityPtr = const Entity*;

    explicit BVH(const std::vector<const Entity*> &entities);

    BVH(const ConstEntityPtr *entities, size_t nEntity);

    bool HasIntersection(const Ray &r) const override;

    bool FindIntersection(const Ray &r, SurfacePoint *sp) const override;

    AABB WorldBound() const override;

    const Material *GetMaterial(const SurfacePoint &sp) const override;

private:

    // Ìî³äentities_ºÍnodes_
    void InitBVH(const ConstEntityPtr *entities, uint32_t nEntity);

    AGZ::ObjArena<> boundArena_;

    std::vector<const Entity*> entities_;
    std::vector<Node> nodes_;
};

AGZ_NS_END(Atrc)
