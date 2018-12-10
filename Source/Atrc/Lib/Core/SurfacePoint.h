#pragma once

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{

class BSDF;
class Entity;

class CoordSystem
{
public:

    Vec3 ex, ey, ez;

    CoordSystem() = default;

    CoordSystem(const Vec3 &ex, const Vec3 &ey, const Vec3 &ez) noexcept;

    static CoordSystem FromEz(const Vec3 &ez) noexcept;

    Vec3 Local2World(const Vec3 &local) const noexcept;

    Vec3 World2Local(const Vec3 &world) const noexcept;

    bool InPositiveHemisphere(const Vec3 &v) const noexcept;
};

struct GeometryIntersection
{
    Real t;               // 相交射线的t值
    Vec3 pos;             // 交点在世界坐标系中的位置
    Vec3 wr;              // 相交射线的反方向
    Vec2 uv;              // 交点在几何体上的参数坐标
    CoordSystem coordSys; // 交点所在曲面的局部坐标系
    uint32_t flag0 = 0;   // 由实体设置的辅助标记，含义由其自行定义
};

// 射线与实体求交的结果
struct Intersection : public GeometryIntersection
{
    const Entity *entity; // 实体指针
};

// Intersection对应的着色信息
struct ShadingPoint
{
    CoordSystem coordSys; // 用于着色计算的局部坐标系
    Vec2 uv;
    const BSDF *bsdf;
};

// ================================= Implementation

inline CoordSystem::CoordSystem(const Vec3 &ex, const Vec3 &ey, const Vec3 &ez) noexcept
    : ex(ex.Normalize()), ey(ey.Normalize()), ez(ez.Normalize())
{

}

inline CoordSystem CoordSystem::FromEz(const Vec3 &ez) noexcept
{
    Vec3 ex;
    if(ApproxEq(Abs(Dot(ez, Vec3::UNIT_X())), Real(1), Real(0.1)))
        ex = Cross(ez, Vec3::UNIT_Y()).Normalize();
    else
        ex = Cross(ez, Vec3::UNIT_X()).Normalize();
    return CoordSystem(ex, Cross(ez, ex).Normalize(), ez.Normalize());
}

inline Vec3 CoordSystem::Local2World(const Vec3 &local) const noexcept
{
    return local.x * ex + local.y * ey + local.z * ez;
}

inline Vec3 CoordSystem::World2Local(const Vec3 &world) const noexcept
{
    return Vec3(Dot(ex, world), Dot(ey, world), Dot(ez, world));
}

inline bool CoordSystem::InPositiveHemisphere(const Vec3 &v) const noexcept
{
    return Dot(ez, v) > 0;
}

} // namespace Atrc
