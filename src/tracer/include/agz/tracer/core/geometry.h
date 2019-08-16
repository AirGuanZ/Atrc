#pragma once

#include <agz/tracer/core/intersection.h>
#include <agz/tracer/utility/object.h>

AGZ_TRACER_BEGIN

/**
 * @brief 世界坐标系中的几何形状
 */
class Geometry : public obj::Object
{
public:

    using Object::Object;

    /**
     * @brief 给定射线是否与该几何体有交点
     */
    virtual bool has_intersection(const Ray &r) const noexcept = 0;

    /**
     * @brief 找到给定射线与该几何体最近的交点
     * 
     * @param r 射线
     * @param inct 交点信息，其每个成员有效当且仅当返回值为true；若返回false，则inct必然未被修改
     * 
     * @return 存在交点时返回true，否则返回false
     */
    virtual bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept = 0;

    /**
     * @brief 返回该实体在世界坐标系中的轴对齐包围盒
     */
    virtual AABB world_bound() const noexcept = 0;

    /**
     * @brief 返回该几何体的可采样表面积
     */
    virtual real surface_area() const noexcept = 0;

    /**
     * @brief 在表面采样一个点，要求表面每个位置的pdf均不为0
     * 
     * @param pdf 采样到返回值的概率密度函数值（w.r.t. surface area）
     */
    virtual SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept = 0;

    /**
     * @brief 返回在表面采样到特定点的pdf（w.r.t. surface area）
     * @param pos 采样到的点的位置
     */
    virtual real pdf(const Vec3 &pos) const noexcept = 0;

    /**
     * @brief 在表面采样一个点，要求从参考点处可见的表面上每个位置的pdf均不为0
     * 
     * @param ref 参考点
     * @param pdf 采样到返回值的概率密度函数值（w.r.t. surface area）
     */
    virtual SurfacePoint sample(const Vec3 &ref, real *pdf, const Sample3 &sam) const noexcept = 0;

    /**
     * @brief 在有参考点的情形下，在表面采样到特定点的pdf（w.r.t. surface area）
     */
    virtual real pdf(const Vec3 &ref, const Vec3 &pos) const noexcept = 0;
};

AGZT_INTERFACE(Geometry)

AGZ_TRACER_END
