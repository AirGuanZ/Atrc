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
    struct InternalTriangle
    {
        Vec3r A, B_A, C_A;
        Vec3r nA, nB_nA, nC_nA;
        Vec2r tA, tB_tA, tC_tA;
    };
    
    struct Node
    {
        bool isLeaf;
        union
        {
            struct
            {
                AABB bound;
                size_t offset;
            } internal;
            
            struct
            {
                size_t startOffset;
                size_t primCount;
            }
        };
    };

    std::vector<InternalTriangle> tris_;
    std::vector<Node> nodes_;
    
    RC<Material> mat_;
    
    static constexpr MAX_LEAF_SIZE = 4;
    
    void InitBVH(const Vetex *vertices, size_t triangleCount);
    
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

    TriangleBVH(const Vertex *vertices, size_t triangleCount, RC<Material> mat);

    ~TriangleBVH();
    
    bool HasIntersection(const Ray &r) const override;
    
    bool EvalIntersection(const Ray &r, Intersection *inct) const override;
    
    AABB GetBoundingBox() const override;

    RC<BxDF> GetBxDF(const Intersection &inct) const override;
};

AGZ_NS_END(Atrc)
