#pragma once

#include <AGZUtils/Config/Config.h>
#include <Atrc/Editor/ResourceInstance/ResourceFactory.h>
#include <Atrc/Editor/ResourceInstance/ResourceInstance.h>

namespace Atrc::Editor
{

template<typename TResource>
class ResourceSaver : public AGZ::Uncopiable
{
    static_assert(std::is_base_of_v<IResource, TResource>);

public:

    using ID = size_t;
    using Ptr = std::shared_ptr<TResource>;

    ResourceSaver()
        : nextNewID_(0)
    {
        
    }

    ID GetResourceID(Ptr p)
    {
        if(auto it = ptr2ID_.find(p); it != ptr2ID_.end())
            return it->second;

        ID ret = nextNewID_++;
        ptr2ID_[p] = ret;
        return ret;
    }

    void Clear()
    {
        ptr2ID_.clear();
        nextNewID_ = 0;
    }

    auto begin() { return ptr2ID_.begin(); }
    auto end()   { return ptr2ID_.end(); }

    auto begin() const { return ptr2ID_.begin(); }
    auto end()   const { return ptr2ID_.end(); }

private:

    std::unordered_map<Ptr, ID> ptr2ID_;
    size_t nextNewID_;
};

template<typename TResource>
class ResourceLoader
{
    static_assert(std::is_base_of_v<IResource, TResource>);

public:

    using ID = typename ResourceSaver<TResource>::ID;
    using Ptr = typename ResourceSaver<TResource>::Ptr;

    template<typename...LoadArgs>
    explicit ResourceLoader(const AGZ::ConfigGroup &resourcePool, const LoadArgs&...loadArgs)
    {
        auto &children = resourcePool.GetChildren();
        for(auto &p : children)
        {
            ID id = AGZ::Parse<ID>(p.first);
            if(ID2Ptr_.find(id) != ID2Ptr_.end())
                std::throw_with_nested(AGZ::Exception("repeated resource id: " + std::to_string(id)));
            ID2Ptr_[id] = RF.CreateAndLoad<TResource>(*p.second, loadArgs...);
        }
    }

    Ptr GetResource(ID id) const
    {
        auto it = ID2Ptr_.find(id);
        if(it != ID2Ptr_.end())
            return it->second;
        std::throw_with_nested(AGZ::Exception("unknown resource id: " + std::to_string(id)));
    }

    void Clear()
    {
        ID2Ptr_.clear();
    }

private:

    std::unordered_map<ID, Ptr> ID2Ptr_;
};

} // namespace Atrc::Editor
