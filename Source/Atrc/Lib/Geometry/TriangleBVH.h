#pragma once

#include <Atrc/Lib/Core/AABB.h>
#include <Atrc/Lib/Core/Geometry.h>

namespace AGZ
{
    class BinarySerializer;
    class BinaryDeserializer;
}

namespace Atrc
{

struct TriangleBVHCoreNode;
struct TriangleBVHCorePrimitive;
struct TriangleBVHCorePrimitiveInfo;

class TriangleBVHCore : public AGZ::Uncopiable
{
public:

    struct Vertex
    {
        Vec3 pos;
        Vec3 nor;
        Vec2 uv;
    };

    TriangleBVHCore(const Vertex *vertices, uint32_t triangleCount);

    TriangleBVHCore(TriangleBVHCore &&moveFrom) noexcept;

    ~TriangleBVHCore();

    bool Serialize(AGZ::BinarySerializer &serializer) const;

    static std::optional<TriangleBVHCore> Deserialize(AGZ::BinaryDeserializer &ds);

    AABB GetLocalBound() const noexcept;

    Real GetSurfaceArea() const noexcept;

    bool HasIntersection(Ray r) const noexcept;

    bool FindIntersection(Ray r, GeometryIntersection *inct) const noexcept;

    Geometry::SampleResult Sample(const Vec3 &sample) const;

private:

    TriangleBVHCore() { }

    void InitBVH(const Vertex *vtx, uint32_t triangleCount);

    std::vector<Real> areaPrefixSum_;

    uint32_t nodeCount_;
    TriangleBVHCoreNode          *nodes_;
    TriangleBVHCorePrimitive     *prims_;
    TriangleBVHCorePrimitiveInfo *primsInfo_;
};

class TriangleBVH : public Geometry
{
    const TriangleBVHCore &core_;

public:

    TriangleBVH(const Transform &local2World, const TriangleBVHCore &core) noexcept;

    bool HasIntersection(const Ray &r) const noexcept override;

    bool FindIntersection(const Ray &r, GeometryIntersection *inct) const noexcept override;

    Real GetSurfaceArea() const noexcept override;

    AABB GetLocalBound() const noexcept override;

    SampleResult Sample(const Vec3 &sample) const noexcept override;

    Real SamplePDF(const Vec3 &pos) const noexcept override;
};

} // namespace Atrc
