#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

struct TriangleBVHData;

class TriangleBVH
    : ATRC_IMPLEMENTS Entity,
      ATRC_PROPERTY AGZ::Uncopiable
{
    TriangleBVHData *data_;
    
public:

    struct Vertex
    {
        Vec3r pos;
        Vec3r nor;
        Vec2r tex;
    };
    
    struct Triangle
    {
        Vertex vertices[3];
    };

    TriangleBVH(const Triangle *triangles, size_t triCount);

    ~TriangleBVH();
    
    bool HasIntersection(const Ray &r) const override;
    
    bool EvalIntersection(const Ray &r, Intersection *inct) const override;
    
    AABB GetBoundingBox() const override;
};

AGZ_NS_END(Atrc)
