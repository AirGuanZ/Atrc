#pragma once

#include <AGZUtils/Utils/Config.h>
#include <AGZUtils/Utils/Misc.h>

namespace Atrc
{

template<typename TResource>
class ResourceData : public AGZ::Uncopiable
{
public:

    virtual ~ResourceData() = default;

    virtual std::unique_ptr<AGZ::ConfigNode> Serialize() const = 0;

    virtual void Deserialize(const AGZ::ConfigGroup &param) = 0;

    virtual TResource *CreateResource() const = 0;
};

template<typename TResource>
class ResourceDataCreator : public AGZ::Uncopiable
{
public:

    virtual ~ResourceDataCreator() = default;

    virtual const std::string &GetTypeName() const = 0;

    virtual ResourceData<TResource> *CreateResourceData() const = 0;
};

} // namespace Atrc
