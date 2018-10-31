#include <limits>
#include <queue>

#include <Atrc/Accelarator/BVH.h>

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

    if(!nodes_[0].isLeaf)
        worldBound_ = *nodes_[0].internal.bound;
    else
    {
        for(uint32_t i = nodes_[0].leaf.start; i < nodes_[0].leaf.end; ++i)
            worldBound_ = worldBound_ | entities_[i]->WorldBound();
    }
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
    return false;
}


AGZ_NS_END(Atrc)
