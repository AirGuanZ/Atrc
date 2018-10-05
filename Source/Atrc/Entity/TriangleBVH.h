#pragma once

#include <vector>

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

struct TriangleBVHData;

class TriangleBVH
    : ATRC_IMPLEMENTS Entity,
      ATRC_PROPERTY AGZ::Uncopiable
{
public:

    static constexpr size_t MAX_LEAF_SIZE = 4;

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

    struct Node
    {
        bool isLeaf;
        union
        {
            struct
            {
                struct
                {
                    struct { Real x, y, z; } low;
                    struct { Real x, y, z; } high;
                } bound;
                size_t offset;
            } internal;

            struct
            {
                size_t startOffset;
                size_t primCount;
            } leaf;
        };
    };

    struct InternalTriangle
    {
        Vec3r A, B_A, C_A;
        Vec3r nA, nB_nA, nC_nA;
        Vec2r tA, tB_tA, tC_tA;
    };

private:

    Real surfaceArea_;
    std::vector<InternalTriangle> tris_;
    std::vector<Node> nodes_;
    
    RC<Material> mat_;
    
    void InitBVH(const Vertex *vertices, size_t triangleCount);
    
public:

    TriangleBVH(const Vertex *vertices, size_t triangleCount, RC<Material> mat);
    
    bool HasIntersection(const Ray &r) const override;
    
    bool EvalIntersection(const Ray &r, Intersection *inct) const override;
    
    AABB GetBoundingBox() const override;

    Real SurfaceArea() const override;

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
