#include <Atrc/Entity/TriangleBVH.h>

AGZ_NS_BEG(Atrc)

namespace
{
    using Vertex = TriangleBVH::Vertex;

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
                size_t startOffset, primCount;
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

    TempNode *FillLeaf(TempNode *ret, size_t start, size_t n)
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

    TempNode *Build(
        MappedTriangles &tris,
        AGZ::SmallObjArena<TempNode> &nodeArena,
        size_t startIdx, size_t endIdx, size_t *nodeCount)
    {
        AGZ_ASSERT(startIdx < endIdx && nodeCount);

        size_t primCount = endIdx - startIdx;
        if(primCount < TriangleBVH::MAX_LEAF_SIZE)
        {
            *nodeCount = 1;
            return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
        }

        // Select the splitting axis
        // If all centriods are the same, create a leaf node

        AABB centroidBound = { tris.GetInfo(startIdx).centroid, tris.GetInfo(startIdx).centroid };
        for(size_t i = startIdx + 1; i < endIdx; ++i)
            centroidBound.Expand(tris.GetInfo(i).centroid);

        Vec3r centroidDelta = centroidBound.high - centroidBound.low;
        Axis splitAxis = SelectAxisWithMaxExtent(centroidDelta);
        if(centroidBound.low[splitAxis] >= centroidBound.high[splitAxis])
        {
            *nodeCount = 1;
            return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
        }

        AABB allBound = Geometry::Triangle::ToBoundingBox(
            tris.GetTriangle(startIdx)[0].pos,
            tris.GetTriangle(startIdx)[1].pos,
            tris.GetTriangle(startIdx)[2].pos);
        for(size_t i = startIdx + 1; i < endIdx; ++i)
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

        for(size_t i = startIdx; i < endIdx; ++i)
        {
            auto n = static_cast<int>((tris.GetInfo(i).centroid - centroidBound.low)[splitAxis]
                / centroidDelta[splitAxis] * BUCKET_COUNT);
            n = Clamp(n, 0, BUCKET_COUNT - 1);

            ++buckets[n].count;
            buckets[n].bound.Expand(tris.GetInfo(i).centroid);
        }

        Real cost[BUCKET_COUNT - 1], invAllBoundArea = 1 / allBound.SurfaceArea();
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

            cost[splitPos] = 0.125 + invAllBoundArea *
                (clow * blow.SurfaceArea() + chigh * bhigh.SurfaceArea());
        }

        int tSplitPos = 0;
        for(int splitPos = 1; splitPos < BUCKET_COUNT - 1; ++splitPos)
        {
            if(cost[splitPos] < cost[tSplitPos])
                tSplitPos = splitPos;
        }

        if(cost[tSplitPos] < primCount)
        {
            *nodeCount = 1;
            return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
        }

        // Split triangles and build two subtrees recursively

        auto midElem = std::partition(
            &tris.triIdxMap[startIdx], &tris.triIdxMap[endIdx],
            [&](size_t idx)
        {
            auto n = static_cast<int>((tris.triInfo[idx].centroid - centroidBound.low)[splitAxis]
                / centroidDelta[splitAxis] * BUCKET_COUNT);
            n = Clamp(n, 0, BUCKET_COUNT - 1);
            return n <= tSplitPos;
        });

        size_t splitIdx = midElem - &tris.triIdxMap[startIdx];
        if(splitIdx == startIdx || splitIdx == endIdx)
        {
            *nodeCount = 1;
            return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
        }

        size_t leftNodeCount = 0, rightNodeCount = 0;
        TempNode *ret = nodeArena.Alloc();
        ret->isLeaf = false;
        ret->internal.bound = allBound;
        ret->internal.left = Build(tris, nodeArena, startIdx, splitIdx, &leftNodeCount);
        ret->internal.right = Build(tris, nodeArena, splitIdx, endIdx, &rightNodeCount);
        *nodeCount = leftNodeCount + rightNodeCount + 1;

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
            TriangleBVH::Node node;
            node.isLeaf = true;
            node.leaf.startOffset = tris.size();
            node.leaf.primCount = tree->leaf.primCount;
            nodes.push_back(node);

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
            TriangleBVH::Node node;
            auto &b = tree->internal.bound;
            node.isLeaf = false;
            node.internal.bound = boundArena.Alloc();
            *node.internal.bound = {
                { b.low.x,  b.low.y,  b.low.z },
                { b.high.x, b.high.y, b.high.z }
            };

            size_t nodeIdx = nodes.size();
            nodes.push_back(node);

            CompactBVHIntoArray(
                nodes, tris, vertices, tree->internal.left, triIdxMap, boundArena);
            
            nodes[nodeIdx].internal.offset = nodes.size();
            CompactBVHIntoArray(
                nodes, tris, vertices, tree->internal.right, triIdxMap, boundArena);
        }
    }
}

/*
    1. 构建链式BVH
    2. 把BVH压缩到数组中，得到nodes_
*/
void TriangleBVH::InitBVH(const Vertex *vertices, size_t triangleCount)
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
        mappedTriangles, tNodeArena, 0, triangleCount - 1, &nodeCount);

    nodes_.clear();
    tris_.clear();
    nodes_.reserve(nodeCount);
    tris_.reserve(triangleCount);
    CompactBVHIntoArray(nodes_, tris_, vertices, root, triIdxMap, boundArena_);
}

bool TriangleBVH::HasIntersectionAux(const Ray &r, size_t nodeIdx) const
{
    AGZ_ASSERT(nodeIdx < nodes_.size());
    const auto &node = nodes_[nodeIdx];
    if(node.isLeaf)
    {
        size_t endOffset = node.leaf.startOffset + node.leaf.primCount;
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

bool TriangleBVH::EvalIntersectionAux(const Ray &r, size_t nodeIdx, Intersection *inct) const
{
    AGZ_ASSERT(nodeIdx < nodes_.size());
    const auto &node = nodes_[nodeIdx];

    if(node.isLeaf)
    {
        bool ret = false;
        size_t endOffset = node.leaf.startOffset + node.leaf.primCount;
        for(size_t i = node.leaf.startOffset; i < endOffset; ++i)
        {
            Intersection tInct;
            const auto &tri = tris_[i];
            if(Geometry::Triangle::EvalIntersection(tri.A, tri.B_A, tri.C_A, r, &tInct)
                && (!ret || tInct.t < inct->t))
            {
                ret = true;
                *inct = tInct;
                inct->flag = i;
                inct->nor = (tri.nA + inct->uv.u * tri.nB_nA + inct->uv.v * tri.nC_nA).Normalize();
            }
        }
        if(ret)
            inct->entity = this;
        return ret;
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

TriangleBVH::TriangleBVH(
    const Vertex *vertices, size_t triangleCount,
    RC<Material> mat, const Transform &local2World)
    : surfaceArea_(0.0), mat_(mat), local2World_(local2World)
{
    InitBVH(vertices, triangleCount);
}

TriangleBVH::TriangleBVH(
    const AGZ::Model::GeometryMesh &mesh, RC<Material> mat, const Transform &local2World)
    : surfaceArea_(0.0), mat_(mat), local2World_(local2World)
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

    InitBVH(vertices.data(), vertices.size() / 3);
}

bool TriangleBVH::HasIntersection(const Ray &r) const
{
    if(nodes_.empty())
        return false;
    return HasIntersectionAux(local2World_.ApplyInverseToRay(r), 0);
}

bool TriangleBVH::EvalIntersection(const Ray &r, Intersection *inct) const
{
    if(nodes_.empty() || !EvalIntersectionAux(local2World_.ApplyInverseToRay(r), 0, inct))
        return false;
    *inct = local2World_.ApplyToIntersection(*inct);
    return true;
}

AABB TriangleBVH::GetBoundingBox() const
{
    if(nodes_.empty())
        return AABB();

    if(nodes_[0].isLeaf)
    {
        AABB ret = Geometry::Triangle::ToBoundingBox(
            tris_[0].A, tris_[0].A + tris_[0].B_A, tris_[0].A + tris_[0].C_A);
        for(size_t i = 1; i < tris_.size(); ++i)
        {
            ret = ret | Geometry::Triangle::ToBoundingBox(
                tris_[i].A, tris_[i].A + tris_[i].B_A, tris_[i].A + tris_[i].C_A);
        }
        return ret;
    }

    return *nodes_[0].internal.bound;
}

Real TriangleBVH::SurfaceArea() const
{
    return surfaceArea_;
}

RC<BxDF> TriangleBVH::GetBxDF(const Intersection &inct) const
{
    const InternalTriangle &tri = tris_[inct.flag];
    return mat_->GetBxDF(inct, tri.tA + tri.tB_tA * inct.uv.u + tri.tC_tA * inct.uv.v);
}

AGZ_NS_END(Atrc)
