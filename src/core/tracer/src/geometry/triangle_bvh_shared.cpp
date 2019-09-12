#include <limits>
#include <queue>
#include <stack>
#include <vector>

#include <agz/tracer/utility/triangle_aux.h>
#include <agz/tracer/utility/path_manager.h>
#include <agz/utility/math/math.h>

#include "./mesh_loader/loader.h"
#include "./transformed_geometry.h"

AGZ_TRACER_BEGIN

namespace
{

    // 用于遍历bvh树的栈大小和空间
    constexpr int TRAVERSAL_STACK_SIZE = 128;
    thread_local uint32_t traversal_stack[TRAVERSAL_STACK_SIZE];

    // triangle bvh中的三角形
    struct Primitive
    {
        Vec3 a_, b_a_, c_a_;
    };

    // triangle bvh中的三角形附加信息
    struct PrimitiveInfo
    {
        Vec3 n_a_, n_b_a_, n_c_a_;
        Vec2 t_a_, t_b_a_, t_c_a_;
        Vec3 x_, z_;
    };

    // triangle bvh节点
    struct Node
    {
        real low[3], high[3];

        // start为uint32_t max时为internal node，否则为leaf node
        uint32_t start, end_or_right_offset;

        static Node new_leaf(const real *low, const real *high, uint32_t start, uint32_t end)
        {
            Node ret = {
                { low[0],  low[1],  low[2] },
				{ high[0], high[1], high[2] },
				start, end
            };
            assert(ret.is_leaf());
            return ret;
        }

        static Node new_interior(const real *low, const real *high, uint32_t right_offset)
        {
            Node ret = {
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
            real nx = inv_dir[0] * (low[0] - ori[0]);
            real ny = inv_dir[1] * (low[1] - ori[1]);
            real nz = inv_dir[2] * (low[2] - ori[2]);

            real fx = inv_dir[0] * (high[0] - ori[0]);
            real fy = inv_dir[1] * (high[1] - ori[1]);
            real fz = inv_dir[2] * (high[2] - ori[2]);

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

    // 建立triangle bvh中临时使用的链式节点
    // left为nullptr时为leaf，否则为internal。BuildingNode的所有权统一由arena管理。
    struct BuildingNode
    {
        AABB bounding;
        BuildingNode *left = nullptr, *right = nullptr;
        uint32_t start = 0, end = 0;
    };

    // 建立triangle bvh中临时使用的三角形信息
    struct BuildingTriangle
    {
        const mesh::Vertex *vtx = nullptr;
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
            BuildingTask task = tasks.front();
            tasks.pop();

            assert(task.start < task.end);

            AABB all_bound, centroid_bound;
            for(uint32_t i = task.start; i < task.end; ++i)
            {
                auto &tri = triangles[i];
                all_bound |= tri.vtx[0].pos;
                all_bound |= tri.vtx[1].pos;
                all_bound |= tri.vtx[2].pos;
                centroid_bound |= tri.centroid;
            }

            // 当三角形数量足够小时构建叶节点
            uint32_t n = task.end - task.start;
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

            // 选择跨度最大的坐标轴为划分方向
            Vec3 centroid_delta = centroid_bound.high - centroid_bound.low;
            int split_axis = centroid_delta[0] > centroid_delta[1] ?
                (centroid_delta[0] > centroid_delta[2] ? 0 : 2) :
                (centroid_delta[1] > centroid_delta[2] ? 1 : 2);

            // 深度较小时使用位置二分，较大时使用数量二分
            uint32_t split_middle;
            if(task.depth < depth_threshold)
            {
                real split_pos = real(0.5) * (centroid_bound.high[split_axis] + centroid_bound.low[split_axis]);
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
            CompactingTask task = tasks.top();
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
                uint32_t start = next_prim_idx;
                uint32_t end = next_prim_idx + (tree->end - tree->start);

                auto &node = node_arr[next_node_idx++];
                node = Node::new_leaf(
                    &tree->bounding.low[0], &tree->bounding.high[0], start, end);

                for(uint32_t i = start, j = tree->start; i < end; ++i, ++j)
                {
                    assert(j < tree->end);

                    auto &tri       = triangles[j];
                    auto &prim      = prim_arr[i];
                    auto &prim_info = prim_info_arr[i];

                    prim.a_   = tri.vtx[0].pos;
                    prim.b_a_ = tri.vtx[1].pos - tri.vtx[0].pos;
                    prim.c_a_ = tri.vtx[2].pos - tri.vtx[0].pos;

                    Vec3 n_a = tri.vtx[0].nor.normalize();
                    Vec3 n_b = tri.vtx[1].nor.normalize();
                    Vec3 n_c = tri.vtx[2].nor.normalize();

                    prim_info.n_a_   = n_a;
                    prim_info.n_b_a_ = n_b - n_a;
                    prim_info.n_c_a_ = n_c - n_a;

                    prim_info.t_a_   = tri.vtx[0].uv;
                    prim_info.t_b_a_ = tri.vtx[1].uv - tri.vtx[0].uv;
                    prim_info.t_c_a_ = tri.vtx[2].uv - tri.vtx[0].uv;

                    prim_info.z_ = cross(prim.b_a_, prim.c_a_).normalize();
                    auto mean_nor = n_a + n_b + n_c;
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

        void initialize(const mesh::Triangle *triangles, uint32_t triangle_count)
        {
            assert(triangles && triangle_count);

            surface_area_ = 0;
            local_bound_ = AABB();

            std::vector<BuildingTriangle> build_triangles(triangle_count);
            for(uint32_t i = 0; i < triangle_count; ++i)
            {
                build_triangles[i].vtx = triangles[i].vtx;
                build_triangles[i].centroid = (triangles[i].vtx[0].pos + triangles[i].vtx[1].pos + triangles[i].vtx[2].pos) / real(3);
                surface_area_ += triangle_area(triangles[i].vtx[1].pos - triangles[i].vtx[0].pos, triangles[i].vtx[2].pos - triangles[i].vtx[0].pos);
                local_bound_ |= triangles[i].vtx[0].pos;
                local_bound_ |= triangles[i].vtx[1].pos;
                local_bound_ |= triangles[i].vtx[2].pos;
            }

            Arena arena;
            auto[root, node_count] = build_bvh(build_triangles.data(), triangle_count, 5, TRAVERSAL_STACK_SIZE / 2, arena);

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
            real inv_dir[3] = { 1 / r.d.x, 1 / r.d.y, 1 / r.d.z };
            real t;

            if(!nodes_[0].has_intersection(&r.o[0], inv_dir, r.t_min, r.t_max, &t))
                return false;

            int top = 0;
            traversal_stack[top++] = 0;

            while(top)
            {
                uint32_t task_node_idx = traversal_stack[--top];
                const Node &node = nodes_[task_node_idx];

                if(node.is_leaf())
                {
                    for(uint32_t i = node.start; i < node.end_or_right_offset; ++i)
                    {
                        auto &prim = prims_[i];
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
            real ori[3]     = { r.o.x,     r.o.y,     r.o.z };
            real inv_dir[3] = { 1 / r.d.x, 1 / r.d.y, 1 / r.d.z };

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
                uint32_t task_node_idx = traversal_stack[--top];
                const Node &node = nodes_[task_node_idx];

                if(node.is_leaf())
                {
                    for(uint32_t i = node.start; i < node.end_or_right_offset; ++i)
                    {
                        auto &prim = prims_[i];
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

                    bool add_left = nodes_[task_node_idx + 1].has_intersection(ori, inv_dir, r.t_min, r.t_max, &t_left);
                    bool add_right = nodes_[node.end_or_right_offset].has_intersection(ori, inv_dir, r.t_min, r.t_max, &t_right);

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

            auto &prim_info = prim_info_[final_prim_idx];

            inct->pos            = r.at(rcd.t_ray);
            inct->geometry_coord = Coord(prim_info.x_, cross(prim_info.z_, prim_info.x_), prim_info.z_);
            inct->uv             = prim_info.t_a_ + rcd.uv.x * prim_info.t_b_a_ + rcd.uv.y * prim_info.t_c_a_;
            inct->t              = rcd.t_ray;

            Vec3 user_z = prim_info.n_a_ + rcd.uv.x * prim_info.n_b_a_ + rcd.uv.y * prim_info.n_c_a_;
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
            auto prim_idx = prim_sampler_.sample(sam.u);
            assert(0 <= prim_idx && static_cast<size_t>(prim_idx) < prims_.size());
            auto &prim = prims_[prim_idx];
            auto &prim_info = prim_info_[prim_idx];

            auto uv = math::distribution::uniform_on_triangle(sam.v, sam.w);

            SurfacePoint spt;
            spt.pos            = prim.a_ + uv.x * prim.b_a_ + uv.y * prim.c_a_;
            spt.geometry_coord = Coord(prim_info.x_, cross(prim_info.z_, prim_info.x_), prim_info.z_);
            spt.uv             = prim_info.t_a_ + uv.x * prim_info.t_b_a_ + uv.y * prim_info.t_c_a_;

            Vec3 user_z = prim_info.n_a_ + uv.x * prim_info.n_b_a_ + uv.y * prim_info.n_c_a_;
            spt.user_coord = spt.geometry_coord.rotate_to_new_z(user_z);

            *pdf = 1 / surface_area_;

            return spt;
        }

        const std::vector<Primitive> &get_prims() const noexcept
        {
            return prims_;
        }

        const AABB &local_bound() const noexcept
        {
            return local_bound_;
        }
    };

}

class TriangleBVHShared : public TransformedGeometry
{
    const UntransformedTriangleBVH *untransformed_ = nullptr;
    AABB world_bound_;

    static const UntransformedTriangleBVH *load(const std::string &filename, Arena &arena)
    {
        AGZ_HIERARCHY_TRY

        auto build_triangles = mesh::load_from_file(filename);
        auto ret = arena.create<UntransformedTriangleBVH>();
        ret->initialize(build_triangles.data(), static_cast<uint32_t>(build_triangles.size()));
        return ret;

        AGZ_HIERARCHY_WRAP("in loading mesh from " + filename)
    }

public:

    using TransformedGeometry::TransformedGeometry;

    static std::string description()
    {
        return R"___(
triangle_bvh(_noembree) [Geometry]
    transform    [Transform[]] transform sequence
    pretransform [0/1] (optional; defaultly set to 0) pretransform contained mesh to [-0.5, 0.5]^3
    filename     [string]      mesh filename
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        init_transform(params);

        static std::map<std::string, const UntransformedTriangleBVH*> filename2bvh;

        auto raw_filename = params.child_str("filename");
        auto filename = init_ctx.path_mgr->get(raw_filename);

        if(auto it = filename2bvh.find(filename); it != filename2bvh.end())
            untransformed_ = it->second;
        else
        {
            auto mesh = load(filename, *init_ctx.arena);
            assert(mesh);
            untransformed_ = mesh;
            filename2bvh[filename] = mesh;
        }

        if(auto node = params.find_child("pretransform"); node && node->as_value().as_int())
        {
            local_to_world_ *= pretransform(untransformed_->local_bound());
            update_ratio();
        }

        world_bound_ = AABB();
        for(auto &prim : untransformed_->get_prims())
        {
            Vec3 local_a = local_to_world_.apply_to_point(prim.a_);
            Vec3 local_b = local_to_world_.apply_to_point(prim.a_ + prim.b_a_);
            Vec3 local_c = local_to_world_.apply_to_point(prim.a_ + prim.c_a_);
            world_bound_ |= local_a;
            world_bound_ |= local_b;
            world_bound_ |= local_c;
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
        Ray local_r = to_local(r);
        return untransformed_->has_intersection(local_r);
    }

    bool closest_intersection(const Ray &r, GeometryIntersection *inct) const noexcept override
    {
        Ray local_r = to_local(r);
        if(!untransformed_->closest_intersection(local_r, inct))
            return false;
        to_world(inct);
        return true;
    }

    AABB world_bound() const noexcept override
    {
        return world_bound_;
    }

    real surface_area() const noexcept override
    {
        return local_to_world_ratio_ * local_to_world_ratio_ * untransformed_->surface_area();
    }

    SurfacePoint sample(real *pdf, const Sample3 &sam) const noexcept override
    {
        auto ret = untransformed_->sample(pdf, sam);
        to_world(&ret);
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

#ifndef USE_EMBREE
AGZT_IMPLEMENTATION(Geometry, TriangleBVH, "triangle_bvh_shared")
#endif

using TriangleBVHNOEmbree = TriangleBVHShared;
AGZT_IMPLEMENTATION(Geometry, TriangleBVHNOEmbree, "triangle_bvh_noembree_shared")

AGZ_TRACER_END
