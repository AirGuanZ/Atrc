#pragma once

#include <Atrc/Lib/Core/Common.h>
#include <Atrc/Lib/Core/CoordSystem.h>
#include <Atrc/Lib/Core/Medium.h>

namespace Atrc
{

class BSDF;
class Entity;

struct GeometryIntersection
{
    Real t;               // 相交射线的t值
    Vec3 pos;             // 交点在世界坐标系中的位置
    Vec3 wr;              // 相交射线的反方向
    Vec2 uv;              // 交点在几何体上的参数坐标
    CoordSystem coordSys; // 交点所在曲面的局部坐标系

    struct
    {
        Vec2 uv;
        CoordSystem coordSys;
    } usr; // 那些可能不由几何体的固有性质决定，而是可以被用户提供的数据调整的量
};

// 射线与实体求交的结果
struct Intersection : GeometryIntersection
{
    const Entity *entity;    // 实体指针
    const Material *material;
    MediumInterface mediumInterface;
};

// Intersection对应的着色信息
struct ShadingPoint
{
    CoordSystem coordSys; // 用于着色计算的局部坐标系
    Vec2 uv;
    const BSDF *bsdf;
};

} // namespace Atrc
