#pragma once

#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/utility/sphere_aux.h>

/*
InfiniteLight以一个包裹整个场景的球形光源的形式存在，一般而言整个场景中至多有一个

InfiniteLightCore负责球体的几何部分，即
    1. 预处理场景信息，计算球心和半径
    2. 计算射线与球体的交点，展现其几何属性\
InfiniteLightImpl负责光照部分，使用时可借助InfiniteLightCore的信息
最后，由InfiniteLightImplToLight/InfiniteLightImplAggregate将一个或多个Impl包装成完全合规的Light，
    再由InfiniteLightEntity将其包装成Entity
*/

AGZ_TRACER_BEGIN

class InfiniteLightCore
{
    real radius_ = 1;
    Vec3 centre_;

    Ray to_local(const Ray &r) const noexcept
    {
        return Ray(r.o - centre_, r.d, r.t_min, r.t_max);
    }

public:

    void preprocess(const Scene &scene)
    {
        auto[low, high] = scene.world_bound();
        real diag_len = (high - low).length();
        radius_ = real(1.1) * diag_len / 2;
        centre_ = real(0.5) * (low + high);
    }

    bool has_intersection(const Ray &r) const
    {
        Ray local_r = to_local(r);
        assert(local_r.o.length() < radius_);
        return sphere::has_intersection(local_r, radius_);
    }

    bool intersection(const Ray &r, GeometryIntersection *inct) const
    {
        Ray local_r = to_local(r);
        assert(local_r.o.length() < radius_);

        real t;
        if(!sphere::closest_intersection(local_r, &t, radius_))
            return false;

        Vec3 pos = local_r.at(t);
        Vec2 geometry_uv(UNINIT);
        Coord geometry_coord(UNINIT);
        sphere::local_geometry_uv_and_coord(pos, &geometry_uv, &geometry_coord, radius_);

        inct->pos            = to_world(pos);
        inct->geometry_coord = -inct->geometry_coord;
        inct->uv             = geometry_uv;
        inct->user_coord     = inct->geometry_coord;
        inct->wr             = -r.d;
        inct->t              = t;
        
        return true;
    }

    SurfacePoint unit_pos_to_spt(const Vec3 &unit_pos) const
    {
        Vec3 local_pos = radius_ * unit_pos;
        Vec3 pos = to_world(local_pos);

        Vec2 geometry_uv(UNINIT);
        Coord geometry_coord(UNINIT);
        sphere::local_geometry_uv_and_coord(local_pos, &geometry_uv, &geometry_coord, radius_);

        SurfacePoint spt;
        spt.pos            = pos;
        spt.geometry_coord = -geometry_coord;
        spt.user_coord     = -geometry_coord;
        spt.uv             = geometry_uv;

        return spt;
    }

    // return t
    bool intersection(const Ray &r, real *t) const
    {
        Ray local_r = to_local(r);
        assert(local_r.o.length() < radius_);

        return sphere::closest_intersection(local_r, t, radius_);
    }

    real world_radius() const noexcept
    {
        return radius_;
    }

    Vec3 to_world(const Vec3 &pnt) const noexcept
    {
        return pnt + centre_;
    }
};

class InfiniteLightImpl : public obj::Object
{
public:

    using Object::Object;

    virtual LightSampleResult sample(const InfiniteLightCore &core, const Vec3 &ref, const Sample4 &sam) const noexcept = 0;

    virtual Spectrum power(const InfiniteLightCore &core) const noexcept = 0;

    virtual Spectrum radiance(const InfiniteLightCore &core, const Vec3 &ref_to_light) const noexcept = 0;

    virtual real pdf(const InfiniteLightCore &core, const Vec3 &ref, const Vec3 &ref_to_light) const noexcept = 0;

    virtual LightEmitResult emit(const InfiniteLightCore &core, const Sample4 &sam) const noexcept = 0;

    virtual void emit_pdf(const InfiniteLightCore &core, const SurfacePoint &spt, const Vec3 &light_to_out, real *pdf_pos, real *pdf_dir) const noexcept = 0;
};

AGZT_INTERFACE(InfiniteLightImpl)

AGZ_TRACER_END
