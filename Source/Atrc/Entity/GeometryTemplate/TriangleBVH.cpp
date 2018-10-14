#include <queue>

#include <Atrc/Entity/GeometryTemplate/TriangleBVH.h>

AGZ_NS_BEG(Atrc)

namespace
{
    using Vertex = TriangleBVH::Vertex;

    constexpr size_t MAX_LEAF_SIZE = 4;

    struct Tri
    {
        size_t id = 0;
        Real surfaceArea = 0.0;
        AABB bounding;
        Vec3r centroid;
    };
    
    struct TempNode
    {
        bool isLeaf;
        union
        {
            struct
            {
                TempNode *left, *right;
                AABB bound;
            } internal;
            
            struct
            {
                uint32_t startOffset, primCount;
            } leaf;
        };
    };
    
    using Axis = uint8_t;
    
    constexpr Axis X = 0;
    constexpr Axis Y = 1;
    constexpr Axis Z = 2;
    
    Axis SelectAxisWithMaxExtent(const Vec3r &delta)
    {
        AGZ_ASSERT(delta.x >= 0 && delta.y >= 0 && delta.z >= 0);
        if(delta.x < delta.y)
            return delta.y < delta.z ? Z : Y;
        return delta.x < delta.z ? Z : X;
    }

    TempNode *FillLeaf(TempNode *ret, uint32_t start, uint32_t n)
    {
        ret->isLeaf = true;
        ret->leaf.startOffset = start;
        ret->leaf.primCount = n;
        return ret;
    }

    struct MappedTriangles
    {
        const Vertex *vertices;
        const std::vector<Tri> &triInfo;
        std::vector<size_t> &triIdxMap;

        MappedTriangles(
            const Vertex *vertices, const std::vector<Tri> &triInfo, std::vector<size_t> &triIdxMap)
            : vertices(vertices), triInfo(triInfo), triIdxMap(triIdxMap)
        {

        }

        const Vertex *GetTriangle(size_t i) const
        {
            AGZ_ASSERT(i < triIdxMap.size());
            return &vertices[3 * triIdxMap[i]];
        }

        const Tri &GetInfo(size_t i) const
        {
            AGZ_ASSERT(i < triInfo.size());
            return triInfo[i];
        }
    };

    struct BuildTask
    {
        uint32_t startIdx, endIdx;
        TempNode **fillNode;
    };

    TempNode *BuildSingleNode(
        MappedTriangles &tris,
        AGZ::SmallObjArena<TempNode> &nodeArena,
        const BuildTask &task,
        size_t *nodeCount,
        std::queue<BuildTask> *tasks)
    {
        uint32_t startIdx = task.startIdx, endIdx = task.endIdx;

        AGZ_ASSERT(startIdx < endIdx && nodeCount && tasks);

        uint32_t primCount = endIdx - startIdx;
        if(primCount <= MAX_LEAF_SIZE)
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
        }

        // Select the splitting axis
        // If all centriods are the same, create a leaf node

        AABB centroidBound = { tris.GetInfo(startIdx).centroid, tris.GetInfo(startIdx).centroid };
        for(auto i : Between(startIdx, endIdx))
            centroidBound.Expand(tris.GetInfo(i).centroid);

        Vec3r centroidDelta = centroidBound.high - centroidBound.low;
        Axis splitAxis = SelectAxisWithMaxExtent(centroidDelta);
        if(centroidBound.low[splitAxis] >= centroidBound.high[splitAxis])
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
        }

        AABB allBound = Geometry::Triangle::ToBoundingBox(
            tris.GetTriangle(startIdx)[0].pos,
            tris.GetTriangle(startIdx)[1].pos,
            tris.GetTriangle(startIdx)[2].pos);
        for(auto i : Between(startIdx + 1, endIdx))
        {
            allBound.Expand(tris.GetTriangle(i)[0].pos);
            allBound.Expand(tris.GetTriangle(i)[1].pos);
            allBound.Expand(tris.GetTriangle(i)[2].pos);
        }

        // Find the best partition position in buckets

        constexpr int BUCKET_COUNT = 16;
        struct Bucket
        {
            int count = 0;
            AABB bound;
        } buckets[BUCKET_COUNT];

        for(auto i : Between(startIdx, endIdx))
        {
            auto n = static_cast<int>((tris.GetInfo(i).centroid[splitAxis] - centroidBound.low[splitAxis])
                / centroidDelta[splitAxis] * BUCKET_COUNT);
            n = Clamp(n, 0, BUCKET_COUNT - 1);

            ++buckets[n].count;
            buckets[n].bound.Expand(tris.GetInfo(i).centroid);
        }

        Real cost[BUCKET_COUNT - 1], invAllBoundArea = 1 / allBound.SurfaceArea();
        for(auto splitPos : Between(0, BUCKET_COUNT - 1))
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

            cost[splitPos] = 0.125 + invAllBoundArea *
                (clow * blow.SurfaceArea() + chigh * bhigh.SurfaceArea());
        }

        int tSplitPos = 0;
        for(auto splitPos : Between(1, BUCKET_COUNT - 1))
        {
            if(cost[splitPos] < cost[tSplitPos])
                tSplitPos = splitPos;
        }

        /*if(cost[tSplitPos] > primCount)
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
        }*/

        // Split triangles and build two subtrees recursively

        std::vector<size_t> leftPartIdx, rightPartIdx;
        for(auto i : Between(startIdx, endIdx))
        {
            auto n = static_cast<int>((tris.GetInfo(i).centroid[splitAxis] - centroidBound.low[splitAxis])
                / centroidDelta[splitAxis] * BUCKET_COUNT);
            n = Clamp(n, 0, BUCKET_COUNT - 1);
            if(n <= tSplitPos)
                leftPartIdx.push_back(tris.triIdxMap[i]);
            else
                rightPartIdx.push_back(tris.triIdxMap[i]);
        }

        uint32_t splitIdx = startIdx + static_cast<uint32_t>(leftPartIdx.size());
        for(size_t i = 0; i < leftPartIdx.size(); ++i)
            tris.triIdxMap[startIdx + i] = leftPartIdx[i];
        for(size_t i = 0; i < rightPartIdx.size(); ++i)
            tris.triIdxMap[splitIdx + i] = rightPartIdx[i];

        if(splitIdx == startIdx || splitIdx == endIdx)
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
        }

        TempNode *ret = nodeArena.Alloc();
        ret->isLeaf = false;
        ret->internal.bound = allBound;

        tasks->push({ startIdx, splitIdx, &ret->internal.left });
        tasks->push({ splitIdx, endIdx, &ret->internal.right });

        *nodeCount += 1;

        return ret;
    }

    TempNode *Build(
        MappedTriangles &tris,
        AGZ::SmallObjArena<TempNode> &nodeArena,
        uint32_t startIdx, uint32_t endIdx,
        size_t *nodeCount)
    {
        *nodeCount = 0;
        TempNode *ret;

        std::queue<BuildTask> tasks;
        tasks.push({ startIdx, endIdx, &ret });

        while(!tasks.empty())
        {
            BuildTask task = tasks.front();
            tasks.pop();
            *task.fillNode = BuildSingleNode(tris, nodeArena, task, nodeCount, &tasks);
        }

        return ret;
    }

    void CompactBVHIntoArray(
        std::vector<TriangleBVH::Node> &nodes,
        std::vector<TriangleBVH::InternalTriangle> &tris,
        const TriangleBVH::Vertex *vertices,
        const TempNode *tree, const std::vector<size_t> &triIdxMap,
        AGZ::SmallObjArena<AABB> &boundArena)
    {
        if(tree->isLeaf)
        {
            nodes.emplace_back(static_cast<uint32_t>(tris.size()), tree->leaf.primCount);

            size_t endOffset = tree->leaf.startOffset + tree->leaf.primCount;
            for(size_t i = tree->leaf.startOffset; i < endOffset; ++i)
            {
                auto *tri = &vertices[3 * triIdxMap[i]];
                TriangleBVH::InternalTriangle intTri;
                intTri.A     = tri[0].pos;
                intTri.B_A   = tri[1].pos - tri[0].pos;
                intTri.C_A   = tri[2].pos - tri[0].pos;
                intTri.nA    = tri[0].nor;
                intTri.nB_nA = tri[1].nor - tri[0].nor;
                intTri.nC_nA = tri[2].nor - tri[0].nor;
                intTri.tA    = tri[0].tex;
                intTri.tB_tA = tri[1].tex - tri[0].tex;
                intTri.tC_tA = tri[2].tex - tri[0].tex;
                tris.push_back(intTri);
            }
        }
        else
        {
            auto pBound = boundArena.Alloc();
            *pBound = tree->internal.bound;
            TriangleBVH::Node node(pBound, 0);

            size_t nodeIdx = nodes.size();
            nodes.push_back(node);

            CompactBVHIntoArray(
                nodes, tris, vertices, tree->internal.left, triIdxMap, boundArena);
            
            nodes[nodeIdx].internal.offset = static_cast<uint32_t>(nodes.size());
            CompactBVHIntoArray(
                nodes, tris, vertices, tree->internal.right, triIdxMap, boundArena);
        }
    }
}

/*
    1. Construct BVH tree in linking-style
    2. Compact the tree into array 'nodes_'
*/
void TriangleBVH::InitBVH(const Vertex *vertices, uint32_t triangleCount)
{
    if(!triangleCount)
        return;

    std::vector<Tri> triInfo(triangleCount);
    std::vector<size_t> triIdxMap(triangleCount);
    for(size_t i = 0, j = 0; i < triangleCount; ++i, j += 3)
    {
        Real sa = Geometry::Triangle::SurfaceArea(
            vertices[j].pos, vertices[j + 1].pos, vertices[j + 2].pos);

        surfaceArea_ += sa;

        triInfo[i].id = i;
        triInfo[i].surfaceArea = sa;
        triInfo[i].bounding = Geometry::Triangle::ToBoundingBox(
            vertices[j].pos, vertices[j + 1].pos, vertices[j + 2].pos);
        triInfo[i].centroid = 0.5 * (triInfo[i].bounding.low + triInfo[i].bounding.high);
    
        triIdxMap[i] = i;
    }
    
    AGZ::SmallObjArena<TempNode> tNodeArena;
    size_t nodeCount = 0;
    MappedTriangles mappedTriangles{ vertices, triInfo, triIdxMap };
    TempNode *root = Build(
        mappedTriangles, tNodeArena, 0, triangleCount, &nodeCount);

    nodes_.clear();
    tris_.clear();
    nodes_.reserve(nodeCount);
    tris_.reserve(triangleCount);
    CompactBVHIntoArray(nodes_, tris_, vertices, root, triIdxMap, boundArena_);
}

bool TriangleBVH::HasIntersectionAux(const Ray &r, uint32_t nodeIdx) const
{
    AGZ_ASSERT(nodeIdx < nodes_.size());
    const auto &node = nodes_[nodeIdx];
    if(node.isLeaf)
    {
        size_t endOffset = node.leaf.endOffset;
        for(size_t i = node.leaf.startOffset; i < endOffset; ++i)
        {
            const auto &tri = tris_[i];
            if(Geometry::Triangle::HasIntersection(tri.A, tri.B_A, tri.C_A, r))
                return true;
        }
        return false;
    }

    if(!node.internal.bound->HasIntersection(r))
        return false;

    return HasIntersectionAux(r, nodeIdx + 1) ||
           HasIntersectionAux(r, node.internal.offset);
}

bool TriangleBVH::EvalIntersectionAux(const Ray &r, uint32_t nodeIdx, Intersection *inct) const
{
    AGZ_ASSERT(nodeIdx < nodes_.size());
    const auto &node = nodes_[nodeIdx];

    if(node.isLeaf)
    {
        Intersection tInct;
        Real bestT = RealT::Infinity();
        uint32_t endOffset = node.leaf.endOffset;
        for(uint32_t i = node.leaf.startOffset; i < endOffset; ++i)
        {
            const auto &tri = tris_[i];
            if(Geometry::Triangle::EvalIntersection(tri.A, tri.B_A, tri.C_A, r, &tInct)
                && tInct.t < bestT)
            {
                *inct = tInct;
                bestT = tInct.t;
                inct->flag = i;
            }
        }
        if(!RealT(bestT).IsInfinity())
        {
            inct->entity = this;
            const auto &tri = tris_[inct->flag];
            inct->nor = (tri.nA + inct->uv.u * tri.nB_nA + inct->uv.v * tri.nC_nA).Normalize();
            return true;
        }
        return false;
    }

    if(!node.internal.bound->HasIntersection(r))
        return false;

    if(EvalIntersectionAux(r, nodeIdx + 1, inct))
    {
        Intersection tInct;
        if(EvalIntersectionAux(r, node.internal.offset, &tInct) && tInct.t < inct->t)
            *inct = tInct;
        return true;
    }

    return EvalIntersectionAux(r, node.internal.offset, inct);
}

TriangleBVH::TriangleBVH(const Vertex *vertices, uint32_t triangleCount)
    : surfaceArea_(0.0)
{
    InitBVH(vertices, triangleCount);
}

TriangleBVH::TriangleBVH(const AGZ::Model::GeometryMesh &mesh)
    : surfaceArea_(0.0)
{
    AGZ_ASSERT(mesh.vertices.size() % 3 == 0);

    auto vertices = mesh.vertices
    | AGZ::Map([](const AGZ::Model::GeometryMesh::Vertex &v)
        {
            Vertex ret;
            ret.pos = v.pos;
            ret.tex = v.tex.uv();
            ret.nor = v.nor;
            return ret;
        })
    | AGZ::Collect<std::vector<Vertex>>();

    InitBVH(vertices.data(), static_cast<uint32_t>(vertices.size()) / 3);
}

bool TriangleBVH::HasIntersection(const Ray &r) const
{
    if(nodes_.empty())
        return false;
    return HasIntersectionAux(r, 0);
}

bool TriangleBVH::EvalIntersection(const Ray &r, Intersection *inct) const
{
    return !nodes_.empty() && EvalIntersectionAux(r, 0, inct);
}

AABB TriangleBVH::GetBoundingBox() const
{
    AABB ret;
    for(auto &t : tris_)
    {
        ret.Expand(t.A);
        ret.Expand(t.A + t.B_A);
        ret.Expand(t.A + t.C_A);
    }
    return ret;
}

Real TriangleBVH::SurfaceArea() const
{
    return surfaceArea_;
}

Vec2r TriangleBVH::GetMaterialParameter(const Intersection &inct) const
{
    const InternalTriangle &tri = tris_[inct.flag];
    return tri.tA + tri.tB_tA * inct.uv.u + tri.tC_tA * inct.uv.v;
}

AGZ_NS_END(Atrc)
