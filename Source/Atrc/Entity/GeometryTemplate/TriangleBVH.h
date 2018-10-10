#pragma once

#include <vector>

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class TriangleBVH
    : ATRC_IMPLEMENTS Entity,
      ATRC_PROPERTY AGZ::Uncopiable
{
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

    struct Node
    {
        Node(const AABB *bound, uint32_t offset)
            : isLeaf(false)
        {
            internal.bound = bound;
            internal.offset = offset;
        }

        Node(uint32_t startOffset, uint32_t primCount)
            : isLeaf(true)
        {
            leaf.startOffset = startOffset;
            leaf.endOffset = startOffset + primCount;
        }

        bool isLeaf;
        union
        {
            struct
            {
                const AABB *bound;
                uint32_t offset;
            } internal;

            struct
            {
                uint32_t startOffset;
                uint32_t endOffset;
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

    AGZ::SmallObjArena<AABB> boundArena_;

    Real surfaceArea_;
    std::vector<InternalTriangle> tris_;
    std::vector<Node> nodes_;
    
    void InitBVH(const Vertex *vertices, uint32_t triangleCount);

    bool HasIntersectionAux(const Ray &r, uint32_t nodeIdx) const;

    bool EvalIntersectionAux(const Ray &r, uint32_t nodeIdx, Intersection *inct) const;
    
public:

    TriangleBVH(const Vertex *vertices, uint32_t triangleCount);

    explicit TriangleBVH(const AGZ::Model::GeometryMesh &mesh);
    
    bool HasIntersection(const Ray &r) const override;
    
    bool EvalIntersection(const Ray &r, Intersection *inct) const override;

    AABB GetBoundingBox() const override;

    Real SurfaceArea() const override;

    Vec2r GetMaterialParameter(const Intersection &inct) const;
};

AGZ_NS_END(Atrc)
