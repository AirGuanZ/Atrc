#pragma once

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

struct LocalCoordSystem
{
    Vec3 ex, ey, ez;

    Vec3 Local2World(const Vec3 &v) const
    {
        return v.x * ex + v.y * ey + v.z * ez;
    }

    Vec3 World2Local(const Vec3 &v) const
    {
        return Vec3(Dot(ex, v), Dot(ey, v), Dot(ez, v));
    }

    static LocalCoordSystem FromEz(const Vec3 &ez)
    {
        Vec3 ex;
        if(ApproxEq(Abs(Dot(ez, Vec3::UNIT_X())), 1.0, 0.1))
            ex = Cross(ez, Vec3::UNIT_Y()).Normalize();
        else
            ex = Cross(ez, Vec3::UNIT_X()).Normalize();
        return { ex, Cross(ez, ex), ez };
    }
};

struct SurfacePoint
{
    // 相交射线的t值
    Real t = 0.0;

    // 表面位置
    Vec3 pos;

    // 相交射线的反方向
    Vec3 wo;

    // 几何uv坐标，由相交几何体的固有性质决定
    Vec2 geoUV;

    // 用户自定义uv坐标，用于纹理映射等
    Vec2 usrUV;

    // 几何局部坐标系，其中ez就是表面法线
    LocalCoordSystem geoLocal;

    // 相交实体
    const Entity *entity = nullptr;
    
    // 含义由实体自行定义，比如用作triangle bvh中的face id等
    uint32_t flag0       = 0;
};

struct ShadingPoint
{
    // 着色局部坐标系，大部分时候与geoLocal一样，受bump mapping等技术的影响
    LocalCoordSystem shdLocal;

    BSDF *bsdf;
};

AGZ_NS_END(Atrc)
