#include <limits>

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
    //InitBVH(entities, static_cast<uint32_t>(nEntity));

    AGZ_ASSERT(!nodes_.empty() && !entities_.empty());

    if(!nodes_[0].isLeaf)
        worldBound_ = *nodes_[0].internal.bound;
    else
    {
        for(uint32_t i = nodes_[0].leaf.start; i < nodes_[0].leaf.end; ++i)
            worldBound_ = worldBound_ | entities_[i]->WorldBound();
    }
}



AGZ_NS_END(Atrc)
