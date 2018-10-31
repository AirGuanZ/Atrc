#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

// TriangleBVH可能有大量的数据，把数据存储分出来便于做instancing
class TriangleBVHCore
{
public:

    struct Vertex
    {
        Vec3 pos;
        Vec3 nor;
        Vec2 uv;
    };

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

    struct InternalTriangle
    {
        Vec3 A, B_A, C_A;
        Vec3 nor;
        Vec2 tA, tB_tA, tC_tA;
        Real surfaceArea = 0.0;
    };

    TriangleBVHCore(const Vertex *vertices, uint32_t triangleCount);

    explicit TriangleBVHCore(const AGZ::Model::GeometryMesh &mesh);

    bool HasIntersection(const Ray &r) const;

    bool FindIntersection(const Ray &r, SurfacePoint *sp) const;

    AABB LocalBound() const;

    Real SurfaceArea() const;

    GeometrySampleResult Sample() const;

    const std::vector<InternalTriangle> &GetAllTriangles() const;

private:

    void InitBVH(const Vertex *vertices, uint32_t triangleCount);

    // 申请Bound用的arena
    AGZ::ObjArena<> boundArena_;

    // triangles_中面积的前缀和，用来二分查找以进行全体三角形上的面积均匀采样
    std::vector<Real> areaPrefixSum_;

    std::vector<InternalTriangle> triangles_;
    std::vector<Node> nodes_;
};

class TriangleBVH : public Geometry
{
    RC<TriangleBVHCore> core_;
    AABB worldBound_;
    Real surfaceArea_;

public:

    TriangleBVH(const Transform &local2World, RC<TriangleBVHCore> bvhCore);

    bool HasIntersection(const Ray &r) const override;

    bool FindIntersection(const Ray &r, SurfacePoint *sp) const override;

    Real SurfaceArea() const override;

    AABB LocalBound() const override;

    AABB WorldBound() const override;

    GeometrySampleResult Sample() const override;

    Real SamplePDF(const Vec3 &pos) const override;

    GeometrySampleResult Sample(const Vec3 &dst) const override;

    Real SamplePDF(const Vec3 &pos, const Vec3 &dst) const override;
};

AGZ_NS_END(Atrc)
