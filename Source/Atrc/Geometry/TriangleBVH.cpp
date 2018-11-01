#include <algorithm>
#include <queue>
#include <stack>
#include <vector>

#include <Atrc/Geometry/TriangleBVH.h>

AGZ_NS_BEG(Atrc)

namespace
{
    Real TriangleSurfaceArea(const Vec3 &B_A, const Vec3 &C_A)
    {
        return 0.5 * Cross(B_A, C_A).Length();
    }

    bool HasIntersectionWithTriangle(const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A, const Ray &r)
    {
        Vec3 s1 = Cross(r.dir, C_A);
        Real div = Dot(s1, B_A);
        if(!div)
            return false;
        Real invDiv = 1 / div;

        Vec3 o_A = r.ori - A;
        Real alpha = Dot(o_A, s1) * invDiv;
        if(alpha < 0 || alpha > 1)
            return false;

        Vec3 s2 = Cross(o_A, B_A);
        Real beta = Dot(r.dir, s2) * invDiv;
        if(beta < 0 || alpha + beta > 1)
            return false;

        Real t = Dot(C_A, s2) * invDiv;
        return r.Between(t);
    }

    // 仅设置sp的t，pos，wo，geoUV
    bool EvalIntersectionWithTriangle(
        const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A,
        const Ray &r, SurfacePoint *sp)
    {
        AGZ_ASSERT(sp);

        Vec3 s1 = Cross(r.dir, C_A);
        Real div = Dot(s1, B_A);
        if(!div)
            return false;
        Real invDiv = 1 / div;

        Vec3 o_A = r.ori - A;
        Real alpha = Dot(o_A, s1) * invDiv;
        if(alpha < 0 || alpha > 1)
            return false;

        Vec3 s2 = Cross(o_A, B_A);
        Real beta = Dot(r.dir, s2) * invDiv;
        if(beta < 0 || alpha + beta > 1)
            return false;

        Real t = Dot(C_A, s2) * invDiv;
        if(!r.Between(t))
            return false;

        sp->t     = t;
        sp->pos   = r.At(t);
        sp->wo    = -r.dir.Normalize();
        sp->geoUV = Vec2(alpha, beta);

        return true;
    }
}

TriangleBVHCore::TriangleBVHCore(const Vertex *vertices, uint32_t triangleCount)
    : areaPrefixSum_(triangleCount)
{
    InitBVH(vertices, triangleCount);

    AGZ_ASSERT(!triangles_.empty());

    areaPrefixSum_[0] = TriangleSurfaceArea(triangles_[0].B_A, triangles_[0].C_A);
    for(uint32_t i = 1; i < triangles_.size(); ++i)
    {
        areaPrefixSum_[i] = areaPrefixSum_[i - 1] +
            TriangleSurfaceArea(triangles_[i].B_A, triangles_[i].C_A);
    }
}

TriangleBVHCore::TriangleBVHCore(const AGZ::Model::GeometryMesh &mesh)
{
    AGZ_ASSERT(mesh.vertices.size() % 3 == 0);

    auto vertices = mesh.vertices
    | AGZ::Map([](const AGZ::Model::GeometryMesh::Vertex &v)
        {
            Vertex ret;
            ret.pos = v.pos;
            ret.uv  = v.tex.uv();
            ret.nor = v.nor;
            return ret;
        })
    | AGZ::Collect<std::vector<Vertex>>();

    auto triangleCount = static_cast<uint32_t>(mesh.vertices.size() / 3);
    areaPrefixSum_.resize(triangleCount);

    InitBVH(vertices.data(), triangleCount);

    AGZ_ASSERT(!triangles_.empty());

    areaPrefixSum_[0] = TriangleSurfaceArea(triangles_[0].B_A, triangles_[0].C_A);
    for(uint32_t i = 1; i < triangles_.size(); ++i)
    {
        areaPrefixSum_[i] = areaPrefixSum_[i - 1] +
            TriangleSurfaceArea(triangles_[i].B_A, triangles_[i].C_A);
    }
}

bool TriangleBVHCore::HasIntersection(const Ray &r) const
{
    AGZ_ASSERT(!nodes_.empty());

    std::queue<uint32_t> tasks;
    tasks.push(0);
    while(!tasks.empty())
    {
        uint32_t taskNodeIdx = tasks.front();
        tasks.pop();
        const Node &node = nodes_[taskNodeIdx];

        bool inct = AGZ::TypeOpr::MatchVar(node,
            [&](const Leaf &leaf)
        {
            for(uint32_t i = leaf.start; i < leaf.end; ++i)
            {
                const auto &tri = triangles_[i];
                if(HasIntersectionWithTriangle(tri.A, tri.B_A, tri.C_A, r))
                    return true;
            }
            return false;
        },
            [&](const Internal &internal)
        {
            if(internal.bound.HasIntersection(r))
            {
                tasks.push(taskNodeIdx + 1);
                tasks.push(internal.rightChild);
            }
            return false;
        });

        if(inct)
            return true;
    }

    return false;
}

bool TriangleBVHCore::FindIntersection(const Ray &r, SurfacePoint *sp) const
{
    AGZ_ASSERT(!nodes_.empty());

    bool ret = false;
    std::queue<uint32_t> tasks;
    tasks.push(0);
    while(!tasks.empty())
    {
        uint32_t taskNodeIdx = tasks.front();
        tasks.pop();
        const Node &node = nodes_[taskNodeIdx];

        AGZ::TypeOpr::MatchVar(node,
            [&](const Leaf &leaf)
        {
            SurfacePoint tSp;
            for(uint32_t i = leaf.start; i < leaf.end; ++i)
            {
                const auto &tri = triangles_[i];
                if(EvalIntersectionWithTriangle(
                    tri.A, tri.B_A, tri.C_A, r, &tSp) && (!ret || tSp.t < sp->t))
                {
                    ret = true;
                    *sp = tSp;
                    sp->flag0 = i;
                }
            }
        },
            [&](const Internal &internal)
        {
            if(internal.bound.HasIntersection(r))
            {
                tasks.push(taskNodeIdx + 1);
                tasks.push(internal.rightChild);
            }
        });
    }

    if(!ret)
        return false;

    const auto &tri = triangles_[sp->flag0];
    sp->usrUV = tri.tA + sp->geoUV.u * tri.tB_tA + sp->geoUV.v * tri.tC_tA;
    sp->geoLocal = LocalCoordSystem::FromEz(tri.nor);
    return true;
}

AABB TriangleBVHCore::LocalBound() const
{
    AGZ_ASSERT(!nodes_.empty());

    return AGZ::TypeOpr::MatchVar(nodes_[0],
        [&](const Leaf &leaf)
    {
        AABB ret;
        for(auto &tri : triangles_)
        {
            ret.Expand(tri.A)
                .Expand(tri.A + tri.B_A)
                .Expand(tri.A + tri.C_A);
        }
        return ret;
    },
        [&](const Internal &internal)
    {
        return internal.bound;
    });
}

Real TriangleBVHCore::SurfaceArea() const
{
    AGZ_ASSERT(!areaPrefixSum_.empty());
    return areaPrefixSum_.back();
}

namespace
{
    struct TriangleInfo
    {
        Real surfaceArea = 0.0;
        Vec3 centroid;
    };

    struct TLeaf;
    struct TInternal;

    using TNode = AGZ::TypeOpr::Variant<TLeaf, TInternal>;

    struct TLeaf
    {
        uint32_t start, end;
    };

    struct TInternal
    {
        TNode *left, *right;
        AABB bound;
    };

    using Axis = uint8_t;
    constexpr Axis X = 0, Y = 1, Z = 2;

    Axis SelectAxisWithMaxExtent(const Vec3 &delta)
    {
        AGZ_ASSERT(delta.x >= 0.0 && delta.y >= 0.0 && delta.z >= 0.0);
        if(delta.x < delta.y)
            return delta.y < delta.z ? Z : Y;
        return delta.x < delta.z ? Z : X;
    }

    TNode *FillLeaf(TNode *ret, uint32_t start, uint32_t end)
    {
        *ret = TLeaf{ start, end };
        return ret;
    }

    struct MappedTriangles
    {
        const TriangleBVHCore::Vertex *vertices;
        const std::vector<TriangleInfo> &triInfo;
        std::vector<uint32_t> &triIdxMap;

        MappedTriangles(
            const TriangleBVHCore::Vertex *vertices, const std::vector<TriangleInfo> &triInfo,
            std::vector<uint32_t> &triIdxMap)
            : vertices(vertices), triInfo(triInfo), triIdxMap(triIdxMap)
        {

        }

        const auto *GetTriangleVertices(size_t i) const
        {
            AGZ_ASSERT(i < triIdxMap.size());
            return &vertices[3 * triIdxMap[i]];
        }

        const auto &GetInfo(size_t i) const
        {
            AGZ_ASSERT(i < triInfo.size());
            return triInfo[i];
        }
    };

    struct BuildTask
    {
        uint32_t start, end;
        TNode **fillNode;
    };

    constexpr uint32_t MAX_LEAF_SIZE = 6;

    TNode *BuildSingleNode(
        MappedTriangles &tris, AGZ::ObjArena<> &nodeArena,
        const BuildTask &task, size_t *nodeCount, std::queue<BuildTask> *taskQueue)
    {
        uint32_t start = task.start, end = task.end;
        AGZ_ASSERT(start < end && nodeCount && taskQueue);

        uint32_t N = end - start;
        if(N <= MAX_LEAF_SIZE)
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Create<TNode>(), start, end);
        }

        // 选择划分轴
        // 若所有centroid都在同一个位置，则创建一个叶节点

        AABB centroidBound;
        for(uint32_t i = start; i < end; ++i)
            centroidBound.Expand(tris.GetInfo(i).centroid);

        Vec3 centroidDelta = centroidBound.high - centroidBound.low;
        Axis splitAxis = SelectAxisWithMaxExtent(centroidDelta);
        if(centroidDelta[splitAxis] <= 0.0)
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Create<TNode>(), start, end);
        }

        // 把所有三角形划分到一堆buckets中，然后从buckets间隙中选择一个最佳划分点

        constexpr int BUCKET_COUNT = 12;

        struct Bucket
        {
            uint32_t count = 0;
            AABB bound;
        } buckets[BUCKET_COUNT];

        auto fac = BUCKET_COUNT / centroidDelta[splitAxis];
        for(uint32_t i = start; i < end; ++i)
        {
            auto &tri = tris.GetInfo(i);

            auto delta = tri.centroid[splitAxis] - centroidBound.low[splitAxis];
            auto iBucket = static_cast<int>(delta * fac);
            iBucket = Clamp(iBucket, 0, BUCKET_COUNT - 1);

            ++buckets[iBucket].count;
            buckets[iBucket].bound.Expand(tri.centroid);
        }

        AABB allBound;
        for(auto i = start; i < end; ++i)
        {
            auto tri = tris.GetTriangleVertices(i);
            allBound.Expand(tri[0].pos);
            allBound.Expand(tri[1].pos);
            allBound.Expand(tri[2].pos);
        }
        Real invAllBoundArea = 1 / allBound.SurfaceArea();

        Real cost[BUCKET_COUNT - 1];
        for(int splitPos = 0; splitPos < BUCKET_COUNT - 1; ++splitPos)
        {
            AABB blow, bhigh;
            int clow = 0, chigh = 0;

            for(int i = 0; i <= splitPos; ++i)
            {
                clow += buckets[i].count;
                blow = blow | buckets[i].bound;
            }

            for(int i = splitPos + 1; i < BUCKET_COUNT; ++i)
            {
                chigh += buckets[i].count;
                bhigh = bhigh | buckets[i].bound;
            }

            cost[splitPos] = 0.125 + invAllBoundArea * (clow * blow.SurfaceArea() + chigh * bhigh.SurfaceArea());
        }

        int splitPos = 0;
        for(int i = 1; i < BUCKET_COUNT - 1; ++i)
        {
            if(cost[i] < cost[splitPos])
                splitPos = i;
        }

        std::vector<uint32_t> left, right;
        for(uint32_t i = start; i < end; ++i)
        {
            auto delta = tris.GetInfo(i).centroid[splitAxis] - centroidBound.low[splitAxis];
            auto iBucket = static_cast<int>(delta * fac);
            iBucket = Clamp(iBucket, 0, BUCKET_COUNT - 1);

            if(iBucket <= splitPos)
                left.push_back(tris.triIdxMap[i]);
            else
                right.push_back(tris.triIdxMap[i]);
        }

        uint32_t splitIdx = start + static_cast<uint32_t>(left.size());
        if(splitIdx == start || splitIdx == end)
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Create<TNode>(), start, end);
        }

        for(uint32_t i = 0; i < left.size(); ++i)
            tris.triIdxMap[start + i] = left[i];
        for(uint32_t i = 0; i < right.size(); ++i)
            tris.triIdxMap[splitIdx + i] = right[i];

        auto *ret = nodeArena.Create<TNode>();
        *ret = TInternal{ nullptr, nullptr, allBound };
        auto &internal = std::get<TInternal>(*ret);

        taskQueue->push({ start, splitIdx, &internal.left });
        taskQueue->push({ splitIdx, end, &internal.right });

        *nodeCount += 1;
        return ret;
    }

    TNode *BuildBVH(
        MappedTriangles &tris, AGZ::ObjArena<> &nodeArena,
        uint32_t start, uint32_t end, size_t *nodeCount)
    {
        *nodeCount = 0;
        TNode *ret = nullptr;

        std::queue<BuildTask> taskQueue;
        taskQueue.push({ start, end, &ret });

        while(!taskQueue.empty())
        {
            BuildTask task = taskQueue.front();
            taskQueue.pop();
            *task.fillNode = BuildSingleNode(tris, nodeArena, task, nodeCount, &taskQueue);
        }

        AGZ_ASSERT(ret);
        return ret;
    }

    struct CompactTask
    {
        const TNode *tree;
        uint32_t *fillNodeIdx;
    };

    void CompactBVHIntoArray(
        std::vector<TriangleBVHCore::Node> &nodes,
        std::vector<TriangleBVHCore::InternalTriangle> &tris,
        const TriangleBVHCore::Vertex *vertices,
        const TNode *root, const std::vector<uint32_t> &triIdxMap)
    {
        std::stack<CompactTask> tasks;
        tasks.push({ root, nullptr });

        while(!tasks.empty())
        {
            auto task = tasks.top();
            tasks.pop();
            auto tree = task.tree;

            if(task.fillNodeIdx)
                *task.fillNodeIdx = static_cast<uint32_t>(nodes.size());

            AGZ::TypeOpr::MatchVar(*tree,
                [&](const TLeaf &leaf)
            {
                auto start = static_cast<uint32_t>(tris.size());
                auto end = start + leaf.end - leaf.start;
                nodes.emplace_back(TriangleBVHCore::Leaf{ start, end });

                for(uint32_t i = leaf.start; i < leaf.end; ++i)
                {
                    auto *tri = &vertices[3 * triIdxMap[i]];
                    TriangleBVHCore::InternalTriangle intTri;
                    intTri.A     = tri[0].pos;
                    intTri.B_A   = tri[1].pos - tri[0].pos;
                    intTri.C_A   = tri[2].pos - tri[0].pos;
                    intTri.nor   = tri[0].nor;
                    intTri.tA    = tri[0].uv;
                    intTri.tB_tA = tri[1].uv - tri[0].uv;
                    intTri.tC_tA = tri[2].uv - tri[0].uv;
                    tris.push_back(intTri);
                }
            },
                [&](const TInternal &internal)
            {
                nodes.emplace_back(TriangleBVHCore::Internal{ internal.bound, 0 });

                tasks.push({ internal.right, &std::get<TriangleBVHCore::Internal>(nodes.back()).rightChild });
                tasks.push({ internal.left, nullptr });
            });
        }
    }
}

void TriangleBVHCore::InitBVH(const Vertex *vertices, uint32_t triangleCount)
{
    if(!triangleCount)
        throw ArgumentException("TriangleBVHCore::InitBVH: triangleCount must be non-zero");

    // BVH树形结构构造

    std::vector<TriangleInfo> triInfo(triangleCount);
    std::vector<uint32_t> triIdxMap(triangleCount);
    for(uint32_t i = 0, j = 0; i < triangleCount; ++i, j += 3)
    {
        auto vtx = &vertices[j];
        Real sa = TriangleSurfaceArea(vtx[1].pos - vtx[0].pos, vtx[2].pos - vtx[0].pos);

        triInfo[i].surfaceArea = sa;
        triInfo[i].centroid = 1.0 / 3 * (vtx[0].pos + vtx[1].pos + vtx[2].pos);

        triIdxMap[i] = i;
    }

    AGZ::ObjArena<> nodeArena;
    size_t nodeCount = 0;
    MappedTriangles tris(vertices, triInfo, triIdxMap);

    TNode *root = BuildBVH(tris, nodeArena, 0, triangleCount, &nodeCount);

    nodes_.clear();
    triangles_.clear();
    nodes_.reserve(nodeCount);
    triangles_.reserve(triangleCount);

    CompactBVHIntoArray(nodes_, triangles_, vertices, root, triIdxMap);
}

GeometrySampleResult TriangleBVHCore::Sample() const
{
    // 在[0, surfaceArea]间生成一个随机数，然后在triangles_面积前缀和中用二分查找选一个三角形
    // 并在三角形上均匀采样

    Real u = AGZ::Math::Random::Uniform(0.0, SurfaceArea());
    auto upper = std::lower_bound(areaPrefixSum_.begin(), areaPrefixSum_.end(), u);

    size_t triIdx = upper == areaPrefixSum_.end() ? (areaPrefixSum_.size() - 1)
                                                  : (upper - areaPrefixSum_.begin());
    auto &tri = triangles_[triIdx];

    Real v = Rand(), w = Rand();
    auto uv = AGZ::Math::DistributionTransform
        ::UniformOnTriangle<Real>::Transform({ v, w });

    GeometrySampleResult ret;
    ret.pos = tri.A + uv.u * tri.B_A + uv.v * tri.C_A;
    ret.nor = tri.nor;
    ret.pdf = 1.0 / SurfaceArea();

    return ret;
}

const std::vector<TriangleBVHCore::InternalTriangle> &TriangleBVHCore::GetAllTriangles() const
{
    return triangles_;
}

TriangleBVH::TriangleBVH(const Transform &local2World, RC<TriangleBVHCore> bvhCore)
    : Geometry(local2World), core_(std::move(bvhCore)), surfaceArea_(0.0)
{
    AGZ_ASSERT(core_);
    for(auto &tri : core_->GetAllTriangles())
    {
        auto A = local2World_.ApplyToPoint(tri.A);
        auto B = local2World_.ApplyToPoint(tri.A + tri.B_A);
        auto C = local2World_.ApplyToPoint(tri.A + tri.C_A);

        worldBound_.Expand(A).Expand(B).Expand(C);
        surfaceArea_ += TriangleSurfaceArea(B - A, C - A);
    }
}

bool TriangleBVH::HasIntersection(const Ray &r) const
{
    return core_->HasIntersection(local2World_.ApplyInverseToRay(r));
}

bool TriangleBVH::FindIntersection(const Ray &r, SurfacePoint *sp) const
{
    AGZ_ASSERT(sp);

    if(!core_->FindIntersection(local2World_.ApplyInverseToRay(r), sp))
        return false;

    sp->pos      = local2World_.ApplyToPoint(sp->pos);
    sp->wo       = local2World_.ApplyToVector(sp->wo).Normalize();
    sp->geoLocal = local2World_.ApplyToCoordSystem(sp->geoLocal);

    return true;
}

Real TriangleBVH::SurfaceArea() const
{
    return surfaceArea_;
}

AABB TriangleBVH::LocalBound() const
{
    return core_->LocalBound();
}

AABB TriangleBVH::WorldBound() const
{
    return worldBound_;
}

GeometrySampleResult TriangleBVH::Sample() const
{
    auto trt = core_->Sample();
    return {
        local2World_.ApplyToPoint(trt.pos),
        local2World_.ApplyToNormal(trt.nor),
        1 / surfaceArea_
    };
}

Real TriangleBVH::SamplePDF(const Vec3 &pos) const
{
    return 1.0 / surfaceArea_;
}

GeometrySampleResult TriangleBVH::Sample(const Vec3 &dst) const
{
    return Sample();
}

Real TriangleBVH::SamplePDF(const Vec3 &pos, const Vec3 &dst) const
{
    return SamplePDF(pos);
}

AGZ_NS_END(Atrc)
