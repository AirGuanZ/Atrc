#include <Atrc/Entity/TriangleBVH.h>

AGZ_NS_BEG(Atrc)

namespace
{
    struct Tri
    {
        size_t id;
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
    
    Axis SelectAxisWithMaxExtent(const AABB &bound)
    {
        Vec3r delta = bound.high - bound.low;
        AGZ_ASSERT(delta.x >= 0 && delta.y >= 0 && delta.z >= 0);
        if(delta.x < delta.y)
            return delta.y < delta.z ? Z : Y;
        return delta.x < delta.z ? Z : X;
    }
}

TriangleBVH(const Vertex *vertices, size_t triangleCount; RC<Material> mat)
    : mat_(mat)
{
    InitBVH(vertices, triangleCount);
}

struct MappedTriangles
{
    const Vertex *vertices;
    const std::vector<Tri> &triInfo;
    std::vector<size_t> &triIdxMap;
    
    const Vertex *GetTriangle(size_t i) const
    {
        AGZ_ASSERT(3 * i < triIdxMap.size());
        return triIdxMap[triIdxMap[3 * i]];
    }
    
    const Tri &GetInfo(size_t i) const
    {
        AGZ_ASSERT(i < triInfo.size());
        return triInfo[i];
    }
    
    void SwapTriangle(size_t i, size_t j)
    {
        AGZ_ASSERT(3 * i < triIdxMap.size() && 3 * j < triIdxMap.size());
        std::swap(triIdxMap[i], triIdxMap[j]);
    }
    
    TempNode *FillLeaf(TempNode *ret, size_t start, size_t n)
    {
        ret->isLeaf = true;
        ret->leaf.startOffset = start;
        ret->leaf.primCount = n;
        return ret;
    }
};

TempNode *Build(
    MappedTriangles &tris,
    SmallObjArena<TempNode> &nodeArena,
    size_t startIdx, size_t endIdx, size_t *nodeCount)
{
    AGZ_ASSERT(startIdx < endIdx && nodeCount);
    
    size_t primCount = endIdx - startIdx;
    
    if(primCount < MAX_LEAF_SIZE)
        return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
    
    // Select the splitting axis
    // If all centriods are the same, create a leaf node
    
    AABB centroidBound = { tris.GetInfo[startIdx].centroid, tris.GetInfo[startIdx].centroid };
    for(size_t i = startIdx + 1; i < endIdx; ++i)
        centroidBound.Expand(tris.GetInfo[i].centroid);
    
    Axis splitAxis = SelectAxisWithMaxExtent(bound);
    if(centroidBound.low[splitAxis] == centroidBound.high[splitAxis])
        return FillLeaf(nodeArena.Alloc(), startIdx, primCount);
    
    // TODO
    return nullptr;
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
        triInfo[i].id = i;
        triInfo[i].bounding = Geometry::Triangle::ToBoundingBox(
            vertices[j].pos, vertices[j + 1].pos, vertices[j + 2].pos);
        triInfo[i].centroid = 0.5 * (triInfo[i].bounding.low + triInfo[i].bounding.high);
    
        triIdxMap[i] = i;
    }
    
    SmallObjArena<TempNode> tNodeArena(32);
    size_t nodeCount = 0;
    
    TempNode *root = Build(
        MappedTriangles{ vertices, triInfo, triIdxMap }, triInfo, tNodeArena,
        0, triangleCount - 1, &nodeCount);
    
    // TODO
}

AGZ_NS_END(Atrc)
