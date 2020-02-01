#include <limits>
#include <queue>
#include <stack>
#include <vector>

#include <agz/tracer/utility/logger.h>
#include <agz/tracer/utility/triangle_aux.h>

#include <agz/utility/mesh.h>
#include <agz/utility/misc.h>

#include "./transformed_geometry.h"

AGZ_TRACER_BEGIN

namespace
{

    // stack for traversal the bvh tree
    constexpr int TRAVERSAL_STACK_SIZE = 128;
    thread_local uint32_t traversal_stack[TRAVERSAL_STACK_SIZE];

    // triangle in bvh
    struct Primitive
    {
        Vec3 a_, b_a_, c_a_;
    };

    // triangle info in bvh
    struct PrimitiveInfo
    {
        Vec3 n_a_, n_b_a_, n_c_a_;
        Vec2 t_a_, t_b_a_, t_c_a_;
        Vec3 x_, z_;
    };

    // node in triangle bvh
    struct Node
    {
        real low[3], high[3];

        // internal node when start == uint32_t.max; otherwise, leaf node
        uint32_t start, end_or_right_offset;

        static Node new_leaf(const real *low, const real *high, uint32_t start, uint32_t end)
        {
            const Node ret = {
                { low[0],  low[1],  low[2] },
				{ high[0], high[1], high[2] },
				start, end
            };
            assert(ret.is_leaf());
            return ret;
        }

        static Node new_interior(const real *low, const real *high, uint32_t right_offset)
        {
            const Node ret = {
                { low[0],  low[1],  low[2] },
				{ high[0], high[1], high[2] },
                std::numeric_limits<uint32_t>::max(),
                right_offset
            };
            assert(!ret.is_leaf());
            return ret;
        }

        bool is_leaf() const noexcept
        {
            return start < std::numeric_limits<uint32_t>::max();
        }

        bool has_intersection(const real *ori, const real *inv_dir, real t_min, real t_max, real *inct_t) const noexcept
        {
            const real nx = inv_dir[0] * (low[0] - ori[0]);
            const real ny = inv_dir[1] * (low[1] - ori[1]);
            const real nz = inv_dir[2] * (low[2] - ori[2]);

            const real fx = inv_dir[0] * (high[0] - ori[0]);
            const real fy = inv_dir[1] * (high[1] - ori[1]);
            const real fz = inv_dir[2] * (high[2] - ori[2]);

            t_min = (std::max)(t_min, (std::min)(nx, fx));
            t_min = (std::max)(t_min, (std::min)(ny, fy));
            t_min = (std::max)(t_min, (std::min)(nz, fz));

            t_max = (std::min)(t_max, (std::max)(nx, fx));
            t_max = (std::min)(t_max, (std::max)(ny, fy));
            t_max = (std::min)(t_max, (std::max)(nz, fz));

            *inct_t = t_min;
            return t_min <= t_max;
        }
    };

    // linking node used in building bvh
    // leaf node when left == nullptr. otherwise, internal node
    struct BuildingNode
    {
        AABB bounding;
        BuildingNode *left = nullptr, *right = nullptr;
        uint32_t start = 0, end = 0;
    };

    // triangle used in building bvh
    struct BuildingTriangle
    {
        const mesh::vertex_t *vtx = nullptr;
        Vec3 centroid;
    };

    struct BuildingResult
    {
        BuildingNode *root;
        uint32_t node_count;
    };

    BuildingResult build_bvh(BuildingTriangle *triangles, uint32_t triangle_count,
                             uint32_t leaf_size_threshold, uint32_t depth_threshold, Arena &arena)
    {
        struct BuildingTask
        {
            BuildingNode **fillback_ptr;
            uint32_t start, end;
            uint32_t depth;
        };

        BuildingResult ret = { nullptr, 0 };

        std::queue<BuildingTask> tasks;
        tasks.push({ &ret.root, 0, triangle_count, 0 });

        while(!tasks.empty())
        {
            const BuildingTask task = tasks.front();
            tasks.pop();

            assert(task.start < task.end);

            AABB all_bound, centroid_bound;
            for(uint32_t i = task.start; i < task.end; ++i)
            {
                auto &tri = triangles[i];
                all_bound |= tri.vtx[0].position;
                all_bound |= tri.vtx[1].position;
                all_bound |= tri.vtx[2].position;
                centroid_bound |= tri.centroid;
            }

            // construct leaf node when triangle count is sufficiently low
            const uint32_t n = task.end - task.start;
            if(n <= leaf_size_threshold)
            {
                ++ret.node_count;

                auto leaf = arena.create<BuildingNode>();
                leaf->bounding = all_bound;
                leaf->left     = nullptr;
                leaf->right    = nullptr;
                leaf->start    = task.start;
                leaf->end      = task.end;

                *task.fillback_ptr = leaf;

                continue;
            }

            // select the split axis with max extent
            const Vec3 centroid_delta = centroid_bound.high - centroid_bound.low;
            const int split_axis = centroid_delta[0] > centroid_delta[1] ?
                (centroid_delta[0] > centroid_delta[2] ? 0 : 2) :
                (centroid_delta[1] > centroid_delta[2] ? 1 : 2);

            // divide the axis with centroid position when recursive depth is small. otherwise, divide with triangle count
            uint32_t split_middle;
            if(task.depth < depth_threshold)
            {
                const real split_pos = real(0.5) * (centroid_bound.high[split_axis] + centroid_bound.low[split_axis]);
                split_middle = task.start;
                for(uint32_t i = task.start; i < task.end; ++i)
                {
                    if(triangles[i].centroid[split_axis] < split_pos)
                        std::swap(triangles[i], triangles[split_middle++]);
                }

                if(split_middle == task.start || split_middle == task.end)
                    split_middle = task.start + n / 2;
            }
            else
            {
                std::sort(
                    triangles + task.start, triangles + task.end,
                    [axis = split_axis](const BuildingTriangle &L, const BuildingTriangle &R)
                { return L.centroid[axis] < R.centroid[axis]; });
                split_middle = task.start + n / 2;
            }

            auto interior = arena.create<BuildingNode>();
            interior->bounding = all_bound;
            interior->left     = nullptr;
            interior->right    = nullptr;
            interior->start    = 0;
            interior->end      = 0;

            *task.fillback_ptr = interior;
            ++ret.node_count;

            tasks.push({ &interior->left, task.start, split_middle, task.depth + 1 });
            tasks.push({ &interior->right, split_middle, task.end, task.depth + 1 });
        }

        return ret;
    }

    void compact_bvh(const BuildingNode *building_node, const BuildingTriangle *triangles,
                     Node *node_arr, Primitive *prim_arr, PrimitiveInfo *prim_info_arr)
    {
        struct CompactingTask
        {
            const BuildingNode *tree;
            uint32_t *fillback_ptr;
        };

        uint32_t next_node_idx = 0, next_prim_idx = 0;

        std::stack<CompactingTask> tasks;
        tasks.push({ building_node, nullptr });

        while(!tasks.empty())
        {
            const CompactingTask task = tasks.top();
            tasks.pop();

            const BuildingNode *tree = task.tree;

            if(task.fillback_ptr)
                *task.fillback_ptr = next_node_idx;

            if(tree->left && tree->right)
            {
                auto &node = node_arr[next_node_idx++];
                node = Node::new_interior(
                    &tree->bounding.low[0], &tree->bounding.high[0], 0);
                tasks.push({ tree->right, &node.end_or_right_offset });
                tasks.push({ tree->left, nullptr });
            }
            else
            {
                const uint32_t start = next_prim_idx;
                const uint32_t end = next_prim_idx + (tree->end - tree->start);

                auto &node = node_arr[next_node_idx++];
                node = Node::new_leaf(
                    &tree->bounding.low[0], &tree->bounding.high[0], start, end);

                for(uint32_t i = start, j = tree->start; i < end; ++i, ++j)
                {
                    assert(j < tree->end);

                    const auto &tri = triangles[j];
                    auto &prim      = prim_arr[i];
                    auto &prim_info = prim_info_arr[i];

                    prim.a_   = tri.vtx[0].position;
                    prim.b_a_ = tri.vtx[1].position - tri.vtx[0].position;
                    prim.c_a_ = tri.vtx[2].position - tri.vtx[0].position;

                    const Vec3 n_a = tri.vtx[0].normal.normalize();
                    const Vec3 n_b = tri.vtx[1].normal.normalize();
                    const Vec3 n_c = tri.vtx[2].normal.normalize();

                    prim_info.n_a_   = n_a;
                    prim_info.n_b_a_ = n_b - n_a;
                    prim_info.n_c_a_ = n_c - n_a;

                    prim_info.t_a_   = tri.vtx[0].tex_coord;
                    prim_info.t_b_a_ = tri.vtx[1].tex_coord - tri.vtx[0].tex_coord;
                    prim_info.t_c_a_ = tri.vtx[2].tex_coord - tri.vtx[0].tex_coord;

                    prim_info.z_ = cross(prim.b_a_, prim.c_a_).normalize();
                    const Vec3 mean_nor = n_a + n_b + n_c;
                    if(dot(mean_nor, prim_info.z_) < 0)
                        prim_info.z_ = -prim_info.z_;

                    prim_info.x_ = dpdu_as_ex(prim.b_a_, prim.c_a_, prim_info.t_b_a_, prim_info.t_c_a_, prim_info.z_);
                }

                next_prim_idx = end;
            }
        }
    }

    // local triangle bvh
    class UntransformedTriangleBVH
    {
        std::vector<Primitive> prims_;
        std::vector<PrimitiveInfo> prim_info_;
        std::vector<Node> nodes_;

        math::distribution::alias_sampler_t<real> prim_sampler_;

        real surface_area_ = 0;
        AABB local_bound_;

    public:

        void initialize(const mesh::triangle_t *triangles, uint32_t triangle_count)
        {
            assert(triangles && triangle_count);

            surface_area_ = 0;
            local_bound_ = AABB();

            std::vector<BuildingTriangle> build_triangles(triangle_count);
            for(uint32_t i = 0; i < triangle_count; ++i)
            {
                build_triangles[i].vtx = triangles[i].vertices;
                build_triangles[i].centroid = (
                    triangles[i].vertices[0].position + triangles[i].vertices[1].position + triangles[i].vertices[2].position) / real(3);
                surface_area_ += triangle_area(
                    triangles[i].vertices[1].position - triangles[i].vertices[0].position,
                    triangles[i].vertices[2].position - triangles[i].vertices[0].position);
                local_bound_ |= triangles[i].vertices[0].position;
                local_bound_ |= triangles[i].vertices[1].position;
                local_bound_ |= triangles[i].vertices[2].position;
            }

            Arena arena;
            auto [root, node_count] = build_bvh(
                build_triangles.data(), triangle_count, 5, TRAVERSAL_STACK_SIZE / 2, arena);

            nodes_.resize(node_count);
            prims_.resize(triangle_count);
            prim_info_.resize(triangle_count);

            compact_bvh(root, build_triangles.data(), nodes_.data(), prims_.data(), prim_info_.data());

            std::vector<real> area_arr(triangle_count);
            for(uint32_t i = 0; i < triangle_count; ++i)
                area_arr[i] = triangle_area(prims_[i].b_a_, prims_[i].c_a_);
            prim_sampler_.initialize(area_arr.data(), static_cast<int>(triangle_count));
        }

        bool has_intersection(const Ray &r) const noexcept
        {
            const real inv_dir[3] = { 1 / r.d.x, 1 / r.d.y, 1 / r.d.z };
            real t;

            if(!nodes_[0].has_intersection(&r.o[0], inv_dir, r.t_min, r.t_max, &t))
                return false;

            int top = 0;
            traversal_stack[top++] = 0;

            while(top)
            {
                const uint32_t task_node_idx = traversal_stack[--top];
                const Node &node = nodes_[task_node_idx];

                if(node.is_leaf())
                {
                    for(uint32_t i = node.start; i < node.end_or_right_offset; ++i)
                    {
                        const Primitive &prim = prims_[i];
                        if(has_intersection_with_triangle(r, prim.a_, prim.b_a_, prim.c_a_))
                            return true;
                    }
                }
                else
                {
                    assert(top + 2 < TRAVERSAL_STACK_SIZE);
                    if(nodes_[task_node_idx + 1].has_intersection(&r.o[0], inv_dir, r.t_min, r.t_max, &t))
                        traversal_stack[top++] = task_node_idx + 1;
                    if(nodes_[node.end_or_right_offset].has_intersection(&r.o[0], inv_dir, r.t_min, r.t_max, &t))
                        traversal_stack[top++] = node.end_or_right_offset;
                }
            }

            return false;
        }

        bool closest_intersection(Ray r, GeometryIntersection *inct) const noexcept
        {
            const real ori[3]     = { r.o.x,     r.o.y,     r.o.z };
            const real inv_dir[3] = { 1 / r.d.x, 1 / r.d.y, 1 / r.d.z };

            int top = 0;
            real tmp_t;
            if(!nodes_[0].has_intersection(ori, inv_dir, r.t_min, r.t_max, &tmp_t))
                return false;

            traversal_stack[top++] = 0;

            TriangleIntersectionRecord rcd, tmp_rcd;
            rcd.t_ray = std::numeric_limits<real>::infinity();
            uint32_t final_prim_idx = 0;

            while(top)
            {
                const uint32_t task_node_idx = traversal_stack[--top];
                const Node &node = nodes_[task_node_idx];

                if(node.is_leaf())
                {
                    for(uint32_t i = node.start; i < node.end_or_right_offset; ++i)
                    {
                        const Primitive &prim = prims_[i];
                        if(closest_intersection_with_triangle(r, prim.a_, prim.b_a_, prim.c_a_, &tmp_rcd))
                        {
                            rcd = tmp_rcd;
                            r.t_max = tmp_rcd.t_ray;
                            final_prim_idx = i;
                        }
                    }
                }
                else
                {
                    real t_left, t_right;

                    const bool add_left = nodes_[task_node_idx + 1].has_intersection(ori, inv_dir, r.t_min, r.t_max, &t_left);
                    const bool add_right = nodes_[node.end_or_right_offset].has_intersection(ori, inv_dir, r.t_min, r.t_max, &t_right);

                    assert(top + 2 <= TRAVERSAL_STACK_SIZE);

                    if(add_left && add_right)
                    {
                        if(t_left < t_right)
                        {
                            traversal_stack[top++] = node.end_or_right_offset;
                            traversal_stack[top++] = task_node_idx + 1;
                        }
                        else
                        {
                            traversal_stack[top++] = task_node_idx + 1;
                            traversal_stack[top++] = node.end_or_right_offset;
                        }
                    }
                    else if(add_left)
                        traversal_stack[top++] = task_node_idx + 1;
                    else
                        traversal_stack[top++] = node.end_or_right_offset;
                }
            }

            if(std::isinf(rcd.t_ray))
                return false;

            const PrimitiveInfo &prim_info = prim_info_[final_prim_idx];

            inct->pos            = r.at(rcd.t_ray);
            inct->geometry_coord = Coord(prim_info.x_, cross(prim_info.z_, prim_info.x_), prim_info.z_);
            inct->uv             = prim_info.t_a_ + rcd.uv.x * prim_info.t_b_a_ + rcd.uv.y * prim_info.t_c_a_;
            inct->t              = rcd.t_ray;

            const Vec3 user_z = prim_info.n_a_ + rcd.uv.x * prim_info.n_b_a_ + rcd.uv.y * prim_info.n_c_a_;
            inct->user_coord = inct->geometry_coord.rotate_to_new_z(user_z);

            inct->wr = -r.d;

            return true;
        }

        real surface_area() const noexcept
        {
            return surface_area_;
        }

        SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept
        {
            const int prim_idx = prim_sampler_.sample(sam.u);
            assert(0 <= prim_idx && static_cast<size_t>(prim_idx) < prims_.size());
            const Primitive &prim = prims_[prim_idx];
            const PrimitiveInfo &prim_info = prim_info_[prim_idx];

            const Vec2 uv = math::distribution::uniform_on_triangle(sam.v, sam.w);

            SurfacePoint spt;
            spt.pos            = prim.a_ + uv.x * prim.b_a_ + uv.y * prim.c_a_;
            spt.geometry_coord = Coord(prim_info.x_, cross(prim_info.z_, prim_info.x_), prim_info.z_);
            spt.uv             = prim_info.t_a_ + uv.x * prim_info.t_b_a_ + uv.y * prim_info.t_c_a_;

            const Vec3 user_z = prim_info.n_a_ + uv.x * prim_info.n_b_a_ + uv.y * prim_info.n_c_a_;
            spt.user_coord = spt.geometry_coord.rotate_to_new_z(user_z);

            *pdf = 1 / surface_area_;

            return spt;
        }

        const std::vector<Primitive> &get_prims() const noexcept
        {
            return prims_;
        }
    };

} // namespace anonymous

class TriangleBVH : public Geometry
{
    std::unique_ptr<const UntransformedTriangleBVH> untransformed_;
    AABB world_bound_;

    static std::unique_ptr<const UntransformedTriangleBVH> load(const std::string &filename, const Transform3 &local_to_world)
    {
        AGZ_HIERARCHY_TRY

        auto build_triangles = mesh::load_from_file(filename);
        for(auto &tri : build_triangles)
        {
            tri.vertices[0].position = local_to_world.apply_to_point(tri.vertices[0].position);
            tri.vertices[1].position = local_to_world.apply_to_point(tri.vertices[1].position);
            tri.vertices[2].position = local_to_world.apply_to_point(tri.vertices[2].position);
            tri.vertices[0].normal = local_to_world.apply_to_vector(tri.vertices[0].normal);
            tri.vertices[1].normal = local_to_world.apply_to_vector(tri.vertices[1].normal);
            tri.vertices[2].normal = local_to_world.apply_to_vector(tri.vertices[2].normal);
        }

        auto ret = std::make_unique<UntransformedTriangleBVH>();
        ret->initialize(build_triangles.data(), static_cast<uint32_t>(build_triangles.size()));

        AGZ_INFO("load triangle mesh from {}", filename);
        AGZ_INFO("triangle count: {}", build_triangles.size());
        return ret;

        AGZ_HIERARCHY_WRAP("in loading mesh from " + filename)
    }

public:

    TriangleBVH(const std::string &filename, const Transform3 &local_to_world)
    {
        AGZ_HIERARCHY_TRY

        untransformed_ = load(filename, local_to_world);

        world_bound_ = AABB();
        for(auto &prim : untransformed_->get_prims())
        {
            world_bound_ |= prim.a_;
            world_bound_ |= prim.a_ + prim.b_a_;
            world_bound_ |= prim.a_ + prim.c_a_;
        }

        for(int i = 0; i != 3; ++i)
        {
            if(world_bound_.low[i] >= world_bound_.high[i])
                world_bound_.low[i] = world_bound_.high[i] - real(0.1) * std::abs(world_bound_.high[i]);
        }

        AGZ_HIERARCHY_WRAP("in initializing triangle_bvh geometry object")
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
        return untransformed_->surface_area();
    }

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override
    {
        *pdf = 1 / surface_area();
        return untransformed_->sample(pdf, sam);
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

std::shared_ptr<Geometry> create_triangle_bvh_noembree(
    const std::string &filename,
    const Transform3 &local_to_world)
{
    return std::make_shared<TriangleBVH>(filename, local_to_world);
}

#ifndef USE_EMBREE

std::shared_ptr<Geometry> create_triangle_bvh(
    const std::string &filename,
    const Transform3 &local_to_world)
{
    return create_triangle_bvh_noembree(filename, local_to_world);
}

#endif

AGZ_TRACER_END
