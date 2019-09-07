#ifdef USE_EMBREE

#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>

#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/utility/embree.h>
#include <agz/tracer/utility/triangle_aux.h>
#include <agz/tracer_utility/math.h>
#include <agz/utility/math.h>
#include <agz/utility/misc.h>

#include "mesh_loader/loader.h"

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

    /**
     * @brief 向embree传递的三角形index格式
     */
    struct EmbreeIndex
    {
        uint32_t v0, v1, v2;
    };

    /**
     * @brief 记录单个三角形的位置
     */
    struct Primitive
    {
        Vec3 a, b_a, c_a;
    };

    /**
     * @brief 记录单个三角形的法线、uv等
     */
    struct PrimitiveInfo
    {
        Vec3 n_a, n_b_a, n_c_a;
        Vec2 t_a, t_b_a, t_c_a;
        Vec3 x, z;
    };

    /**
     * @brief 将embree的上一个错误信息作为异常抛出
     */
    [[noreturn]] void throw_embree_error()
    {
        RTCError err = rtcGetDeviceError(embree_device());
        throw ObjectConstructionException(embree_err_str(err));
    }

    /**
     * @brief 将一个几何物体封装进embree scene中
     */
    class UntransformedTriangleBVH : public misc::uncopyable_t
    {
        RTCScene scene_ = nullptr;
        unsigned int geo_id_ = 0;

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

        void initialize(const mesh::Triangle *triangles, size_t triangle_count)
        {
            assert(triangles && triangle_count > 0);
            assert(!scene_);

            surface_area_ = 0;
            local_bound_ = AABB();

            // 0. 准备embree的vertex buffer和index buffer
            // 1. 遍历所有三角形，填充prob buffer、vertex buffer和prim info buffer

            RTCDevice device = embree_device();
            scene_ = rtcNewScene(device);
            if(!scene_)
                throw_embree_error();

            RTCGeometry mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
            if(!mesh)
                throw_embree_error();
            AGZ_SCOPE_GUARD({ rtcReleaseGeometry(mesh); });

            size_t vertex_count = triangle_count * 3;
            auto vertices = static_cast<EmbreeVertex*>(rtcSetNewGeometryBuffer(
                mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(EmbreeVertex), vertex_count));
            if(!vertices)
                throw_embree_error();

            auto indices = static_cast<EmbreeIndex*>(rtcSetNewGeometryBuffer(
                mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(EmbreeIndex), triangle_count));
            if(!indices)
                throw_embree_error();

            prim_info_.reserve(triangle_count);
            prims_.reserve(triangle_count);
            std::vector<real> areas;
            areas.reserve(triangle_count);

            for(size_t i = 0, j = 0; i < triangle_count; ++i, j += 3)
            {
                auto &triangle = triangles[i].vtx;

                for(int k = 0; k < 3; ++k)
                {
                    vertices[j + k].x = triangle[k].pos.x;
                    vertices[j + k].y = triangle[k].pos.y;
                    vertices[j + k].z = triangle[k].pos.z;
                    vertices[j + k].r = 1;
                }

                prims_.push_back({
                    triangle[0].pos,
                    triangle[1].pos - triangle[0].pos,
                    triangle[2].pos - triangle[0].pos
                    });

                indices[i].v0 = static_cast<uint32_t>(j + 0);
                indices[i].v1 = static_cast<uint32_t>(j + 1);
                indices[i].v2 = static_cast<uint32_t>(j + 2);

                Vec3 n_a = triangle[0].nor.normalize();
                Vec3 n_b = triangle[1].nor.normalize();
                Vec3 n_c = triangle[2].nor.normalize();

                PrimitiveInfo info;
                info.n_a = n_a;
                info.n_b_a = n_b - n_a;
                info.n_c_a = n_c - n_a;

                info.t_a = triangle[0].uv;
                info.t_b_a = triangle[1].uv - triangle[0].uv;
                info.t_c_a = triangle[2].uv - triangle[0].uv;

                Vec3 b_a = triangle[1].pos - triangle[0].pos;
                Vec3 c_a = triangle[2].pos - triangle[0].pos;
                info.z = cross(b_a, c_a).normalize();
                Vec3 mean_nor = n_a + n_b + n_c;
                if(dot(info.z, mean_nor) < 0)
                    info.z = -info.z;
                info.x = dpdu_as_ex(b_a, c_a, info.t_b_a, info.t_c_a, info.z);

                prim_info_.push_back(info);

                real area = triangle_area(b_a, c_a);
                areas.push_back(area);
                surface_area_ += area;

                local_bound_ |= triangle[0].pos;
                local_bound_ |= triangle[1].pos;
                local_bound_ |= triangle[2].pos;
            }

            prim_sampler_.initialize(areas.data(), static_cast<int>(areas.size()));

            rtcCommitGeometry(mesh);
            geo_id_ = rtcAttachGeometry(scene_, mesh);
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
            return ray.tfar < 0 && isinf(ray.tfar);
        }

        bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept
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

            real t_val = rayhit.ray.tfar;
            real u = rayhit.hit.u;
            real v = rayhit.hit.v;
            auto &info = prim_info_[rayhit.hit.primID];

            inct->pos = r.at(t_val);
            inct->geometry_coord = Coord(info.x, cross(info.z, info.x), info.z);
            inct->uv = info.t_a + u * info.t_b_a + v * info.t_c_a;
            inct->t = t_val;

            Vec3 user_z = info.n_a + u * info.n_b_a + v * info.n_c_a;
            inct->user_coord = inct->geometry_coord.rotate_to_new_z(user_z);

            inct->wr = -r.d;

            return true;
        }

        SurfacePoint uniformly_sample(const Sample3 &sam) const noexcept
        {
            int prim_idx = prim_sampler_.sample(sam.u);
            assert(0 <= prim_idx && static_cast<size_t>(prim_idx) < prims_.size());
            auto &prim = prims_[prim_idx];
            auto &prim_info = prim_info_[prim_idx];

            auto uv = math::distribution::uniform_on_triangle(sam.v, sam.w);

            SurfacePoint spt;
            spt.pos = prim.a + uv.x * prim.b_a + uv.y * prim.c_a;
            spt.geometry_coord = Coord(prim_info.x, cross(prim_info.z, prim_info.x), prim_info.z);
            spt.uv = prim_info.t_a + uv.x * prim_info.t_b_a + uv.y * prim_info.t_c_a;

            Vec3 user_z = prim_info.n_a + uv.x * prim_info.n_b_a + uv.y * prim_info.n_c_a;
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

class TriangleBVHEmbreeWS : public Geometry
{
    const tri_bvh_embree_ws::UntransformedTriangleBVH *untransformed_ = nullptr;
    AABB world_bound_;

    real surface_area_ = 0;

    static const tri_bvh_embree_ws::UntransformedTriangleBVH *load(const std::string &filename, const Transform3 &local_to_world, Arena &arena)
    {
        AGZ_HIERARCHY_TRY

        auto build_triangles = mesh::load_from_file(filename);
        for(auto &tri : build_triangles)
        {
            tri.vtx[0].pos = local_to_world.apply_to_point(tri.vtx[0].pos);
            tri.vtx[1].pos = local_to_world.apply_to_point(tri.vtx[1].pos);
            tri.vtx[2].pos = local_to_world.apply_to_point(tri.vtx[2].pos);
            tri.vtx[0].nor = local_to_world.apply_to_vector(tri.vtx[0].nor);
            tri.vtx[1].nor = local_to_world.apply_to_vector(tri.vtx[1].nor);
            tri.vtx[2].nor = local_to_world.apply_to_vector(tri.vtx[2].nor);
        }

        auto ret = arena.create<tri_bvh_embree_ws::UntransformedTriangleBVH>();
        ret->initialize(build_triangles.data(), build_triangles.size());
        return ret;

        AGZ_HIERARCHY_WRAP("in loading mesh from " + filename)
    }

public:

    using Geometry::Geometry;

    static std::string description()
    {
        return R"___(
triangle_bvh(_embree) [Geometry]
    transform [Transform[]] transform sequence
    filename  [string]      mesh filename
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        auto raw_filename = params.child_str("filename");
        auto filename = init_ctx.path_mgr->get(raw_filename);

        Transform3 local_to_world = params.child_transform3("transform");
        auto mesh = load(filename, local_to_world, *init_ctx.arena);
        assert(mesh);
        untransformed_ = mesh;

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
                world_bound_.low[i] = world_bound_.high[i] - real(0.1) * std::abs(world_bound_.high[i]);
        }

        AGZ_HIERARCHY_WRAP("in initializing triangle_bvh_embree_ws geometry object")
    }

    bool has_intersection(const Ray &r) const noexcept override
    {
        return untransformed_->has_intersection(r);
    }

    bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept override
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
        auto ret = untransformed_->uniformly_sample(sam);
        *pdf = 1 / surface_area();
        return ret;
    }

    SurfacePoint sample(const Vec3&, real *pdf, const Sample3 &sam) const noexcept override
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

using TriangleBVHEmbreeWS2 = TriangleBVHEmbreeWS;

AGZT_IMPLEMENTATION(Geometry, TriangleBVHEmbreeWS, "triangle_bvh")
AGZT_IMPLEMENTATION(Geometry, TriangleBVHEmbreeWS2, "triangle_bvh_embree")

AGZ_TRACER_END

#endif // #ifdef USE_EMBREE
