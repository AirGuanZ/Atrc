#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

#include <agz/tracer/utility/config.h>
#include <agz/utility/misc/exception.h>

AGZ_TRACER_BEGIN

class Aggregate;
class Camera;
class Entity;
class NonareaLight;
class Film;
class FilmFilter;
class Fresnel;
class Geometry;
class Material;
class Medium;
class PathTracingIntegrator;
class PostProcessor;
class Renderer;
class ProgressReporter;
class Sampler;
class Scene;
class Texture;

AGZ_TRACER_END

AGZ_TRACER_FACTORY_BEGIN

class CreatingContext;

class CreatingObjectException : public std::runtime_error
{
public:

    using runtime_error::runtime_error;
};

template<typename T>
class Creator
{
public:

    virtual ~Creator() = default;

    virtual std::string name() const = 0;

    virtual std::shared_ptr<T> create(const ConfigGroup &params, CreatingContext &context) const = 0;
};

template<typename T>
class Factory
{
    std::string factory_name_;

    std::unordered_map<std::string, std::unique_ptr<Creator<T>>> name2creator_;

public:

    explicit Factory(std::string factory_name);

    void add_creator(std::unique_ptr<Creator<T>> &&creator);

    const Creator<T> *get_creator(const std::string &name) const;

    const std::string &name() const noexcept;
};

class PathMapper
{
public:

    virtual ~PathMapper() = default;

    virtual std::string map(const std::string &path) const = 0;
};

class CreatingContext
{
    template<typename...Types>
    using FactoryTuple = std::tuple<Factory<Types>...>;

    FactoryTuple<
        Aggregate,
        Camera,
        Entity,
        NonareaLight,
        Film,
        FilmFilter,
        Fresnel,
        Geometry,
        Material,
        Medium,
        PathTracingIntegrator,
        PostProcessor,
        Renderer,
        ProgressReporter,
        Sampler,
        Scene,
        Texture> factory_tuple_;

public:

    CreatingContext();

    const PathMapper *path_mapper;
    const ConfigGroup *reference_root;

    template<typename T>
    Factory<T> &factory() noexcept;

    template<typename T>
    const Factory<T> &factory() const noexcept;

    template<typename T>
    std::shared_ptr<T> create(const std::string &type_name, const ConfigGroup &params, CreatingContext &context);

    template<typename T>
    std::shared_ptr<T> create(const ConfigGroup &params, CreatingContext &context);

    template<typename T>
    std::shared_ptr<T> create(const std::string &type_name, const ConfigGroup &params);

    template<typename T>
    std::shared_ptr<T> create(const ConfigGroup &params);
};

template<typename T>
Factory<T>::Factory(std::string factory_name)
    : factory_name_(std::move(factory_name))
{
    class ReferenceCreator : public Creator<T>
    {
        mutable std::map<std::vector<std::string>, std::shared_ptr<T>> name2obj_;

    public:

        std::string name() const override
        {
            return "reference";
        }

        std::shared_ptr<T> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            AGZ_HIERARCHY_TRY

            auto &name_arr = params.child_array("name");
            if(name_arr.size() < 1)
                throw CreatingObjectException("empty reference name sequence");

            std::vector<std::string> names;
            names.reserve(name_arr.size());
            for(size_t i = 0; i < name_arr.size(); ++i)
                names.push_back(name_arr.at(i).as_value().as_str());

            if(auto it = name2obj_.find(names); it != name2obj_.end())
                return it->second;

            const ConfigGroup *group = context.reference_root;
            for(size_t i = 0; i < names.size(); ++i)
                group = &group->child_group(names[i]);

            auto &true_params = group->child_group(names.back());
            auto ret = context.create<T>(true_params, context);
            name2obj_[names] = ret;

            return ret;

            AGZ_HIERARCHY_WRAP("in creating referenced object")
        }
    };

    this->add_creator(std::make_unique<ReferenceCreator>());
}

template<typename T>
void Factory<T>::add_creator(std::unique_ptr<Creator<T>> &&creator)
{
    auto name = creator->name();
    name2creator_[name] = std::move(creator);
}

template<typename T>
const Creator<T> *Factory<T>::get_creator(const std::string &name) const
{
    auto it = name2creator_.find(name);
    return it != name2creator_.end() ? it->second.get() : nullptr;
}

template<typename T>
const std::string &Factory<T>::name() const noexcept
{
    return factory_name_;
}

template<typename T>
Factory<T> &CreatingContext::factory() noexcept
{
    return std::get<Factory<T>>(factory_tuple_);
}

template<typename T>
const Factory<T> &CreatingContext::factory() const noexcept
{
    return std::get<Factory<T>>(factory_tuple_);
}

template<typename T>
std::shared_ptr<T> CreatingContext::create(const ConfigGroup &params, CreatingContext &context)
{
    return this->create<T>(params.child_str("type"), params, context);
}

template<typename T>
std::shared_ptr<T> CreatingContext::create(const std::string &type_name, const ConfigGroup &params, CreatingContext &context)
{
    AGZ_HIERARCHY_TRY

    auto creator = this->factory<T>().get_creator(type_name);
    if(!creator)
        throw CreatingObjectException("unknown creator type name: " + type_name);

    {
        AGZ_HIERARCHY_TRY
        return creator->create(params, context);
        AGZ_HIERARCHY_WRAP("in creating object with creator: " + creator->name())
    }

    AGZ_HIERARCHY_WRAP("in creating object with factory: " + this->factory<T>().name())
}

template<typename T>
std::shared_ptr<T> CreatingContext::create(const std::string &type_name, const ConfigGroup &params)
{
    return this->create<T>(type_name, params, *this);
}

template<typename T>
std::shared_ptr<T> CreatingContext::create(const ConfigGroup &params)
{
    return this->create<T>(params, *this);
}

AGZ_TRACER_FACTORY_END
