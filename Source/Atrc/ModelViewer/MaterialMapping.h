#pragma once

#include <memory>

class MaterialMappingInstance
{
public:

    virtual ~MaterialMappingInstance() = default;

    virtual void Display() = 0;
};

class MaterialMappingCreator
{
public:

    virtual ~MaterialMappingCreator() = default;

    virtual const char *GetName() const = 0;

    virtual std::shared_ptr<MaterialMappingInstance> CreateInstance() const = 0;
};
