#pragma once

#include <Atrc/Lib/Core/AABB.h>
#include <Atrc/Lib/Core/Geometry.h>

namespace Atrc
{

struct TriangleBVHNode;

class TriangleBVHCore
{
public:

    struct Vertex
    {
        Vec3 pos;
        Vec3 nor;
        Vec2 uv;
    };

    // 创建一个空BVH，调用其Deserialize之外的任何成员函数都是非法的
    TriangleBVHCore() = default;

    TriangleBVHCore(const Vertex *vertices, uint32_t triangleCount);

    TriangleBVHCore(TriangleBVHCore &&moveFrom) noexcept;

    bool Serialize(AGZ::BinaryDeserializer &serializer) const;

    bool Deserialize(AGZ::BinaryDeserializer &deserializer) const;

    AABB GetLocalBound() const noexcept;

    bool HasIntersection(const Ray &r) const noexcept;

    bool FindIntersection(const Ray &r, GeometryIntersection *inct) const noexcept;

    Geometry::SampleResult Sample() const;

private:

    void InitBVH(const Vertex *vtx, uint32_t triangleCount);

    AABB localBound_;
    TriangleBVHNode *nodes_;
};

} // namespace Atrc
