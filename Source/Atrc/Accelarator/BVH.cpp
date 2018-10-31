#include <limits>
#include <queue>
#include <stack>

#include <Atrc/Accelarator/BVH.h>
#include <Atrc/Material/UncallableMaterial.h>

AGZ_NS_BEG(Atrc)

BVH::BVH(const std::vector<const Entity*> &entities)
    : BVH(entities.data(), entities.size())
{

}

BVH::BVH(const ConstEntityPtr *entities, size_t nEntity)
{
    if(!nEntity)
        throw ArgumentException("BVH::BVH: nEntity must be non-zero");
    if(nEntity > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
        throw ArgumentException("BVH::BVH: nEntity is too large");
    InitBVH(entities, static_cast<uint32_t>(nEntity));

    AGZ_ASSERT(!nodes_.empty() && !entities_.empty());
}

bool BVH::HasIntersection(const Ray &r) const
{
    AGZ_ASSERT(!nodes_.empty());

    std::queue<uint32_t> tasks;
    tasks.push(0);
    while(!tasks.empty())
    {
        uint32_t taskNodeIdx = tasks.front();
        tasks.pop();
        const Node &node = nodes_[taskNodeIdx];

        if(node.isLeaf)
        {
            for(uint32_t i = node.leaf.start; i < node.leaf.end; ++i)
            {
                if(entities_[i]->HasIntersection(r))
                    return true;
            }
        }
        else if(node.internal.bound->HasIntersection(r))
        {
            tasks.push(taskNodeIdx + 1);
            tasks.push(node.internal.rightChild);
        }
    }

    return false;
}

bool BVH::FindIntersection(const Ray &r, SurfacePoint *sp) const
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

        if(node.isLeaf)
        {
            SurfacePoint tSp;
            for(uint32_t i = node.leaf.start; i < node.leaf.end; ++i)
            {
                const auto &ent = entities_[i];
                if(ent->FindIntersection(r, &tSp) && (!ret || tSp.t < sp->t))
                {
                    ret = true;
                    *sp = tSp;
                }
            }
        }
        else if(node.internal.bound->HasIntersection(r))
        {
            tasks.push(taskNodeIdx + 1);
            tasks.push(node.internal.rightChild);
        }
    }

    return ret;
}

AABB BVH::WorldBound() const
{
    AGZ_ASSERT(!nodes_.empty());

    if(nodes_[0].isLeaf)
    {
        AABB ret;
        for(uint32_t i = nodes_[0].leaf.start; i < nodes_[0].leaf.end; ++i)
            ret = ret | entities_[i]->WorldBound();
        return ret;
    }

    return *nodes_[0].internal.bound;
}

const Material *BVH::GetMaterial(const SurfacePoint &sp) const
{
    return &STATIC_UNCALLABLE_MATERIAL;
}

namespace
{
    struct EntityInfo
    {
        const Entity *entity = nullptr;
        AABB bound;
        Vec3 centroid;
    };

    struct TNode
    {
        TNode() { }

        bool isLeaf;
        union
        {
            struct
            {
                TNode *left, *right;
                AABB bound;
            } internal;

            struct
            {
                uint32_t start, end;
            } leaf;
        };
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
        ret->isLeaf = true;
        ret->leaf.start = start;
        ret->leaf.end = end;
        return ret;
    }

    struct BuildTask
    {
        uint32_t start, end;
        TNode **fillNode;
    };

    constexpr uint32_t MAX_LEAF_SIZE = 3;

    TNode *BuildSingleNode(
        std::vector<EntityInfo> &ents, AGZ::ObjArena<> &nodeArena,
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

        AABB centroidBound;
        for(uint32_t i = start; i < end; ++i)
            centroidBound.Expand(ents[i].centroid);

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
            auto &ent = ents[i];

            auto delta = ent.centroid[splitAxis] - centroidBound.low[splitAxis];
            auto iBucket = static_cast<int>(delta * fac);
            iBucket = Clamp(iBucket, 0, BUCKET_COUNT - 1);

            ++buckets[iBucket].count;
            buckets[iBucket].bound.Expand(ent.centroid);
        }

        AABB allBound;
        for(auto i = start; i < end; ++i)
            allBound = allBound | ents[i].bound;
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

        if(cost[splitPos] > N)
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Create<TNode>(), start, end);
        }

        std::vector<EntityInfo> left, right;
        for(uint32_t i = start; i < end; ++i)
        {
            auto delta = ents[i].centroid[splitAxis] - centroidBound.low[splitAxis];
            auto iBucket = static_cast<int>(delta * fac);
            iBucket = Clamp(iBucket, 0, BUCKET_COUNT - 1);

            if(iBucket <= splitPos)
                left.push_back(ents[i]);
            else
                right.push_back(ents[i]);
        }

        uint32_t splitIdx = start + static_cast<uint32_t>(left.size());
        if(splitIdx == start || splitIdx == end)
        {
            *nodeCount += 1;
            return FillLeaf(nodeArena.Create<TNode>(), start, end);
        }

        for(uint32_t i = 0; i < left.size(); ++i)
            ents[start + i] = left[i];
        for(uint32_t i = 0; i < right.size(); ++i)
            ents[splitIdx + i] = right[i];

        auto *ret = nodeArena.Create<TNode>();
        ret->isLeaf = false;
        ret->internal.bound = allBound;

        taskQueue->push({ start, splitIdx, &ret->internal.left });
        taskQueue->push({ splitIdx, end, &ret->internal.right });

        *nodeCount += 1;
        return ret;
    }

    TNode *BuildBVH(
        std::vector<EntityInfo> &ents, AGZ::ObjArena<> &nodeArena,
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
            *task.fillNode = BuildSingleNode(ents, nodeArena, task, nodeCount, &taskQueue);
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
        std::vector<BVH::Node> &nodes,
        const std::vector<EntityInfo> &entInfo,
        std::vector<const Entity*> &ents,
        const TNode *root,
        AGZ::ObjArena<> &boundArena)
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

            if(tree->isLeaf)
            {
                BVH::Node node;
                node.isLeaf = true;
                node.leaf.start = static_cast<uint32_t>(ents.size());
                node.leaf.end = node.leaf.start + tree->leaf.end - tree->leaf.start;
                nodes.push_back(node);

                for(uint32_t i = tree->leaf.start; i < tree->leaf.end; ++i)
                    ents.push_back(entInfo[i].entity);
            }
            else
            {
                auto pBound = boundArena.Create<AABB>();
                *pBound = tree->internal.bound;

                BVH::Node node;
                node.isLeaf = false;
                node.internal.bound = pBound;
                node.internal.rightChild = 0;

                size_t nodeIdx = nodes.size();
                nodes.push_back(node);

                tasks.push({ tree->internal.right, &nodes[nodeIdx].internal.rightChild });
                tasks.push({ tree->internal.left, nullptr });
            }
        }
    }
}

void BVH::InitBVH(const ConstEntityPtr *entities, uint32_t nEntity)
{
    AGZ_ASSERT(entities && nEntity);

    std::vector<EntityInfo> entInfo(nEntity);
    for(uint32_t i = 0; i < nEntity; ++i)
    {
        entInfo[i].entity = entities[i];
        entInfo[i].bound = entities[i]->WorldBound();
        entInfo[i].centroid = 0.5 * (entInfo[i].bound.high + entInfo[i].bound.low);
    }

    AGZ::ObjArena<> nodeArena;
    size_t nodeCount = 0;

    TNode *root = BuildBVH(entInfo, nodeArena, 0, nEntity, &nodeCount);
    AGZ_ASSERT(nodes_.empty() && entities_.empty());
    nodes_.reserve(nodeCount);
    entities_.reserve(nEntity);

    CompactBVHIntoArray(nodes_, entInfo, entities_, root, boundArena_);
}

AGZ_NS_END(Atrc)
