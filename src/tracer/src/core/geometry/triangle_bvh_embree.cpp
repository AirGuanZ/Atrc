#ifdef USE_EMBREE

#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>

#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/utility/embree.h>
#include <agz/tracer/utility/logger.h>
#include <agz/tracer/utility/triangle_aux.h>
#include <agz/utility/mesh.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

namespace tri_bvh_embree_ws
{

    std::string embree_err_str(RTCError err)
    {
        switch(err)
        {
        case RTC_ERROR_NONE:              return "no error occurred";
        case RTC_ERROR_UNKNOWN:           return "an unknown error has occurred";
        case RTC_ERROR_INVALID_ARGUMENT:  return "an invalid argument was specified";
        case RTC_ERROR_INVALID_OPERATION: return "the operation is not allowed for the specified object";
        case RTC_ERROR_OUT_OF_MEMORY:     return "there is not enough memory left to complete the operation";
        case RTC_ERROR_UNSUPPORTED_CPU:   return "the CPU is not supported as it does not support the lowest ISA Embree is compiled for";
        case RTC_ERROR_CANCELLED:         return "the operation got canceled by a memory monitor callback or progress monitor callback function";
        default:                          return "unknown embree error code: " + std::to_string(err);
        }
    }

    /**
     * @brief 向embree传递的顶点格式
     */
    struct EmbreeVertex
    {
        float x, y, z, r;
    };

    struct EmbreeIndex
    {
        uint32_t v0, v1, v2;
    };

    struct Primitive
    {
        Vec3 a, b_a, c_a;
    };

    struct PrimitiveInfo
    {
        Vec3 n_a, n_b_a, n_c_a;
        Vec2 t_a, t_b_a, t_c_a;
        Vec3 x, z;
    };

    [[noreturn]] void throw_embree_error()
    {
        const RTCError err = rtcGetDeviceError(embree_device());
        throw ObjectConstructionException(embree_err_str(err));
    }

    class UntransformedTriangleBVH : public misc::uncopyable_t
    {
        RTCScene scene_ = nullptr;
        unsigned int geo_id_  = 0;

        std::vector<Primitive> prims_;
        std::vector<PrimitiveInfo> prim_info_;

        math::distribution::alias_sampler_t<real> prim_sampler_;

        real surface_area_ = 0;
        AABB local_bound_;

    public:

        ~UntransformedTriangleBVH()
        {
            if(scene_)
                rtcReleaseScene(scene_);
        }

        void initialize(const mesh::triangle_t *triangles, size_t triangle_count)
        {
            assert(triangles && triangle_count > 0);
            assert(!scene_);

            surface_area_ = 0;
            local_bound_ = AABB();

            // 0. prepare embree的vertex buffer and index buffer
            // 1. fill prob buffer、vertex buffer and prim info buffer

            RTCDevice device = embree_device();
            scene_ = rtcNewScene(device);
            if(!scene_)
                throw_embree_error();

            RTCGeometry mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
            if(!mesh)
                throw_embree_error();
            AGZ_SCOPE_GUARD({ rtcReleaseGeometry(mesh); });

            const size_t vertex_count = triangle_count * 3;
            const auto vertices = static_cast<EmbreeVertex*>(
                rtcSetNewGeometryBuffer(
                    mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
                    sizeof(EmbreeVertex), vertex_count));
            if(!vertices)
                throw_embree_error();

            const auto indices = static_cast<EmbreeIndex*>(rtcSetNewGeometryBuffer(
                mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                sizeof(EmbreeIndex), triangle_count));
            if(!indices)
                throw_embree_error();

            prim_info_.reserve(triangle_count);
            prims_.reserve(triangle_count);
            std::vector<real> areas;
            areas.reserve(triangle_count);

            for(size_t i = 0, j = 0; i < triangle_count; ++i, j += 3)
            {
                auto &triangle = triangles[i].vertices;

                for(int k = 0; k < 3; ++k)
                {
                    vertices[j + k].x = triangle[k].position.x;
                    vertices[j + k].y = triangle[k].position.y;
                    vertices[j + k].z = triangle[k].position.z;
                    vertices[j + k].r = 1;
                }

                prims_.push_back({
                    triangle[0].position,
                    triangle[1].position - triangle[0].position,
                    triangle[2].position - triangle[0].position
                });

                indices[i].v0 = static_cast<uint32_t>(j + 0);
                indices[i].v1 = static_cast<uint32_t>(j + 1);
                indices[i].v2 = static_cast<uint32_t>(j + 2);

                const Vec3 n_a = triangle[0].normal.normalize();
                const Vec3 n_b = triangle[1].normal.normalize();
                const Vec3 n_c = triangle[2].normal.normalize();

                PrimitiveInfo info;
                info.n_a = n_a;
                info.n_b_a = n_b - n_a;
                info.n_c_a = n_c - n_a;

                info.t_a = triangle[0].tex_coord;
                info.t_b_a = triangle[1].tex_coord - triangle[0].tex_coord;
                info.t_c_a = triangle[2].tex_coord - triangle[0].tex_coord;

                const Vec3 b_a = triangle[1].position - triangle[0].position;
                const Vec3 c_a = triangle[2].position - triangle[0].position;
                info.z = cross(b_a, c_a).normalize();
                const Vec3 mean_nor = n_a + n_b + n_c;
                if(dot(info.z, mean_nor) < 0)
                    info.z = -info.z;
                info.x = dpdu_as_ex(b_a, c_a, info.t_b_a, info.t_c_a, info.z);

                prim_info_.push_back(info);

                const real area = triangle_area(b_a, c_a);
                areas.push_back(area);
                surface_area_ += area;

                local_bound_ |= triangle[0].position;
                local_bound_ |= triangle[1].position;
                local_bound_ |= triangle[2].position;
            }

            prim_sampler_.initialize(areas.data(), static_cast<int>(areas.size()));

            rtcSetGeometryBuildQuality(mesh, RTC_BUILD_QUALITY_HIGH);
            rtcCommitGeometry(mesh);

            geo_id_ = rtcAttachGeometry(scene_, mesh);
            
            rtcSetSceneBuildQuality(scene_, RTC_BUILD_QUALITY_HIGH);
            rtcCommitScene(scene_);
        }

        bool has_intersection(const Ray &r) const noexcept
        {
            RTCRay ray = {
                r.o.x, r.o.y, r.o.z,
                r.t_min,
                r.d.x, r.d.y, r.d.z,
                0,
                r.t_max,
                static_cast<unsigned>(-1), 0, 0
            };

            RTCIntersectContext inct_ctx{};
            rtcInitIntersectContext(&inct_ctx);
            rtcOccluded1(scene_, &inct_ctx, &ray);
            return ray.tfar < 0 && std::isinf(ray.tfar);
        }

        bool closest_intersection(
            const Ray &r, GeometryIntersection *inct) const noexcept
        {
            alignas(16) RTCRayHit rayhit = {
            {
                r.o.x, r.o.y, r.o.z,
                r.t_min,
                r.d.x, r.d.y, r.d.z,
                0,
                r.t_max,
                static_cast<unsigned>(-1), 0, 0
            }, { } };

            rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
            rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
            rayhit.hit.primID = RTC_INVALID_GEOMETRY_ID;

            RTCIntersectContext inct_ctx{};
            rtcInitIntersectContext(&inct_ctx);
            rtcIntersect1(scene_, &inct_ctx, &rayhit);
            if(rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
                return false;

            const real t_val = rayhit.ray.tfar;
            const real u = rayhit.hit.u;
            const real v = rayhit.hit.v;
            const PrimitiveInfo &info = prim_info_[rayhit.hit.primID];

            inct->pos = r.at(t_val);
            inct->geometry_coord = Coord(info.x, cross(info.z, info.x), info.z);
            inct->uv = info.t_a + u * info.t_b_a + v * info.t_c_a;
            inct->t = t_val;

            const Vec3 user_z = info.n_a + u * info.n_b_a + v * info.n_c_a;
            inct->user_coord = inct->geometry_coord.rotate_to_new_z(user_z);

            inct->wr = -r.d;

            return true;
        }

        SurfacePoint uniformly_sample(const Sample3 &sam) const noexcept
        {
            const int prim_idx = prim_sampler_.sample(sam.u);
            assert(0 <= prim_idx && static_cast<size_t>(prim_idx) < prims_.size());
            const Primitive &prim = prims_[prim_idx];
            const PrimitiveInfo &prim_info = prim_info_[prim_idx];

            auto uv = math::distribution::uniform_on_triangle(sam.v, sam.w);

            SurfacePoint spt;

            spt.pos = prim.a + uv.x * prim.b_a + uv.y * prim.c_a;

            spt.geometry_coord = Coord(
                prim_info.x, cross(prim_info.z, prim_info.x), prim_info.z);

            spt.uv = prim_info.t_a + uv.x * prim_info.t_b_a + uv.y * prim_info.t_c_a;

            const Vec3 user_z = prim_info.n_a + uv.x * prim_info.n_b_a
                                              + uv.y * prim_info.n_c_a;

            spt.user_coord = spt.geometry_coord.rotate_to_new_z(user_z);

            return spt;
        }

        const std::vector<Primitive> &get_prims() const noexcept
        {
            return prims_;
        }

        real surface_area() const noexcept
        {
            return surface_area_;
        }

        const AABB &local_bound() const noexcept
        {
            return local_bound_;
        }
    };

} // namespace tri_bvh_embree

class TriangleBVHEmbree : public Geometry
{
    Box<const tri_bvh_embree_ws::UntransformedTriangleBVH> untransformed_;
    AABB world_bound_;

    real surface_area_ = 0;

    static Box<const tri_bvh_embree_ws::UntransformedTriangleBVH> load(
        std::vector<mesh::triangle_t> build_triangles,
        const Transform3 &local_to_world)
    {
        for(auto &tri : build_triangles)
        {
            tri.vertices[0].position = local_to_world.apply_to_point(
                tri.vertices[0].position);
            tri.vertices[1].position = local_to_world.apply_to_point(
                tri.vertices[1].position);
            tri.vertices[2].position = local_to_world.apply_to_point(
                tri.vertices[2].position);
            tri.vertices[0].normal   = local_to_world.apply_to_vector(
                tri.vertices[0].normal);
            tri.vertices[1].normal   = local_to_world.apply_to_vector(
                tri.vertices[1].normal);
            tri.vertices[2].normal   = local_to_world.apply_to_vector(
                tri.vertices[2].normal);
        }

        auto ret = newBox<tri_bvh_embree_ws::UntransformedTriangleBVH>();
        ret->initialize(build_triangles.data(), build_triangles.size());

        return ret;
    }

public:

    TriangleBVHEmbree(
        std::vector<mesh::triangle_t> build_triangles,
        const Transform3 &local_to_world)
    {
        AGZ_HIERARCHY_TRY

        untransformed_ = load(std::move(build_triangles), local_to_world);

        world_bound_ = AABB();
        surface_area_ = 0;
        for(auto &prim : untransformed_->get_prims())
        {
            world_bound_ |= prim.a;
            world_bound_ |= prim.a + prim.b_a;
            world_bound_ |= prim.a + prim.c_a;
            surface_area_ += triangle_area(prim.b_a, prim.c_a);
        }

        for(int i = 0; i != 3; ++i)
        {
            if(world_bound_.low[i] >= world_bound_.high[i])
                world_bound_.low[i] = world_bound_.high[i] -
                                    real(0.1) * std::abs(world_bound_.high[i]);
        }

        AGZ_HIERARCHY_WRAP("in initializing triangle_bvh_embree_ws geometry object")
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        return untransformed_->has_intersection(r);
    }

    bool closest_intersection(
        const Ray &r, GeometryIntersection *inct) const noexcept override
    {
        return untransformed_->closest_intersection(r, inct);
    }

    AABB world_bound() const noexcept override
    {
        return world_bound_;
    }

    real surface_area() const noexcept override
    {
        return surface_area_;
    }

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override
    {
        *pdf = 1 / surface_area();
        return untransformed_->uniformly_sample(sam);
    }

    SurfacePoint sample(
        const Vec3&, real *pdf, const Sample3 &sam) const noexcept override
    {
        return sample(pdf, sam);
    }

    real pdf(const Vec3&) const noexcept override
    {
        return 1 / surface_area();
    }

    real pdf(const Vec3&, const Vec3 &sample) const noexcept override
    {
        return pdf(sample);
    }
};

RC<Geometry> create_triangle_bvh_embree(
    std::vector<mesh::triangle_t> build_triangles,
    const Transform3 &local_to_world)
{
    return newRC<TriangleBVHEmbree>(std::move(build_triangles), local_to_world);
}

RC<Geometry> create_triangle_bvh(
    std::vector<mesh::triangle_t> build_triangles,
    const Transform3 &local_to_world)
{
    return create_triangle_bvh_embree(std::move(build_triangles), local_to_world);
}

AGZ_TRACER_END

#endif // #ifdef USE_EMBREE
