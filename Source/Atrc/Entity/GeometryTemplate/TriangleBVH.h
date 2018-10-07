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
        Node(const AABB *bound, size_t offset)
            : isLeaf(false)
        {
            internal.bound = bound;
            internal.offset = offset;
        }

        Node(size_t startOffset, size_t primCount)
            : isLeaf(true)
        {
            leaf.startOffset = startOffset;
            leaf.primCount = primCount;
        }

        bool isLeaf;
        union
        {
            struct
            {
                const AABB *bound;
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

    AGZ::SmallObjArena<AABB> boundArena_;

    Real surfaceArea_;
    std::vector<InternalTriangle> tris_;
    std::vector<Node> nodes_;

    Transform local2World_;
    
    void InitBVH(const Vertex *vertices, size_t triangleCount);

    bool HasIntersectionAux(const Ray &r, size_t nodeIdx) const;

    bool EvalIntersectionAux(const Ray &r, size_t nodeIdx, Intersection *inct) const;
    
public:

    TriangleBVH(const Vertex *vertices, size_t triangleCount, const Transform &local2World);

    TriangleBVH(const AGZ::Model::GeometryMesh &mesh, const Transform &local2World);
    
    bool HasIntersection(const Ray &r) const override;
    
    bool EvalIntersection(const Ray &r, Intersection *inct) const override;

    AABB GetBoundingBox() const override;

    Real SurfaceArea() const override;

    Vec2r GetMaterialParameter(const Intersection &inct) const;
};

AGZ_NS_END(Atrc)
