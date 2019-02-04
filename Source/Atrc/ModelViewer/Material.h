#pragma once

#include <memory>

class MaterialInstance
{
    std::string name_;

public:

    explicit MaterialInstance(std::string name) : name_(std::move(name)) { }

    virtual ~MaterialInstance() = default;

    virtual void Display() = 0;

    const std::string &GetName() const noexcept { return name_; }
};

class MaterialCreator
{
public:

    virtual ~MaterialCreator() = default;

    virtual const char *GetName() const = 0;

    virtual std::shared_ptr<MaterialInstance> CreateInstance() const = 0;
};
