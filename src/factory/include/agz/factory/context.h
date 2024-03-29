#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>

#include <agz/tracer/utility/config.h>
#include <agz-utils/misc.h>
#include <agz-utils/string.h>

AGZ_TRACER_FACTORY_BEGIN

#define AGZ_FACTORY_WORKING_DIR_PATH_NAME "${working-directory}"
#define AGZ_FACTORY_SCENE_DESC_PATH_NAME  "${scene-directory}"

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

    virtual RC<T> create(
        const ConfigGroup &params, CreatingContext &context) const = 0;
};

template<>
class Creator<Camera>
{
public:

    virtual ~Creator() = default;

    virtual std::string name() const = 0;

    virtual RC<Camera> create(
        const ConfigGroup &params, CreatingContext &context,
        real film_aspect) const = 0;
};

template<typename T>
class Factory
{
    std::string factory_name_;

    std::unordered_map<std::string, Box<Creator<T>>> name2creator_;

public:

    explicit Factory(std::string factory_name);

    void add_creator(Box<Creator<T>> &&creator);

    const Creator<T> *get_creator(const std::string &name) const;

    const std::string &name() const noexcept;
};

class PathMapper
{
public:

    virtual ~PathMapper() = default;

    virtual std::string map(const std::string &path) const = 0;
};

class BasicPathMapper : public PathMapper
{
    std::map<std::string, std::string> replacers_;

public:

    void add_replacer(const std::string &key, const std::string &value);

    std::string map(const std::string &s) const override;
};

class CreatingContext
{
    template<typename...Types>
    using FactoryTuple = std::tuple<Factory<Types>...>;

    FactoryTuple<
        Aggregate,
        Camera,
        Entity,
        EnvirLight,
        FilmFilter,
        Geometry,
        Material,
        Medium,
        PostProcessor,
        Renderer,
        RendererInteractor,
        Scene,
        Texture2D,
        Texture3D> factory_tuple_;

public:

    CreatingContext();

    const PathMapper *path_mapper;
    const ConfigGroup *reference_root;

    template<typename T>
    Factory<T> &factory() noexcept;

    template<typename T>
    const Factory<T> &factory() const noexcept;

    template<typename T, typename...Args>
    RC<T> create(const ConfigGroup &params, Args&&...args);
};

template<typename T>
class ReferenceCreator : public Creator<T>
{
    mutable std::map<std::vector<std::string>, RC<T>> name2obj_;

public:

    std::string name() const override { return "reference"; }

    RC<T> create(
        const ConfigGroup &params, CreatingContext &context) const override;
};

template<>
class ReferenceCreator<Camera> : public Creator<Camera>
{
    mutable std::map<std::vector<std::string>, RC<Camera>> name2obj_;

public:

    std::string name() const override { return "reference"; }

    RC<Camera> create(
        const ConfigGroup &params, CreatingContext &context,
        real film_aspect) const override;
};

inline void BasicPathMapper::add_replacer(
    const std::string &key, const std::string &value)
{
    replacers_[key] = value;
}

inline std::string BasicPathMapper::map(const std::string &s) const
{
    std::string ret(s);
    bool done = false;
    while(!done)
    {
        done = true;
        for(auto &p : replacers_)
        {
            if(stdstr::replace_(ret, p.first, p.second) != 0)
                done = false;
        }
    }
    return absolute(std::filesystem::path(ret)).lexically_normal().string();
}

template<typename T>
RC<T> ReferenceCreator<T>::create(
    const ConfigGroup &params, CreatingContext &context) const
{
    AGZ_HIERARCHY_TRY

    const ConfigArray &name_arr = params.child_array("name");
    if(name_arr.size() < 1)
        throw CreatingObjectException("empty reference name sequence");

    std::vector<std::string> names;
    names.reserve(name_arr.size());
    for(size_t i = 0; i < name_arr.size(); ++i)
        names.push_back(name_arr.at(i).as_value().as_str());

    if(auto it = name2obj_.find(names); it != name2obj_.end())
        return it->second;

    const ConfigGroup *group = context.reference_root;
    for(size_t i = 0; i < names.size() - 1; ++i)
        group = &group->child_group(names[i]);

    const ConfigGroup &true_params = group->child_group(names.back());
    auto ret = context.create<T>(true_params);
    name2obj_[names] = ret;

    return ret;

    AGZ_HIERARCHY_WRAP("in creating referenced object")
}

inline RC<Camera> ReferenceCreator<Camera>::create(
    const ConfigGroup &params, CreatingContext &context,
    real film_aspect) const
{
    AGZ_HIERARCHY_TRY

    const ConfigArray &name_arr = params.child_array("name");
    if(name_arr.size() < 1)
        throw CreatingObjectException("empty reference name sequence");

    std::vector<std::string> names;
    names.reserve(name_arr.size());
    for(size_t i = 0; i < name_arr.size(); ++i)
        names.push_back(name_arr.at(i).as_value().as_str());

    if(auto it = name2obj_.find(names); it != name2obj_.end())
        return it->second;

    const ConfigGroup *group = context.reference_root;
    for(size_t i = 0; i < names.size() - 1; ++i)
        group = &group->child_group(names[i]);

    const ConfigGroup &true_params = group->child_group(names.back());
    auto ret = context.create<Camera>(true_params, film_aspect);
    name2obj_[names] = ret;

    return ret;

    AGZ_HIERARCHY_WRAP("in creating referenced object")
}

template<typename T>
Factory<T>::Factory(std::string factory_name)
    : factory_name_(std::move(factory_name))
{
    this->add_creator(newBox<ReferenceCreator<T>>());
}

template<typename T>
void Factory<T>::add_creator(Box<Creator<T>> &&creator)
{
    const std::string name = creator->name();
    name2creator_[name] = std::move(creator);
}

template<typename T>
const Creator<T> *Factory<T>::get_creator(const std::string &name) const
{
    const auto it = name2creator_.find(name);
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

template<typename T, typename...Args>
RC<T> CreatingContext::create(const ConfigGroup &params, Args&&...args)
{
    AGZ_HIERARCHY_TRY

    const std::string &type_name = params.child_str("type");
    const Creator<T> *creator = this->factory<T>().get_creator(type_name);
    if(!creator)
        throw CreatingObjectException(
            "unknown creator type name: " + type_name);

    {
        AGZ_HIERARCHY_TRY
        return creator->create(params, *this, std::forward<Args>(args)...);
        AGZ_HIERARCHY_WRAP(
            "in creating object with creator: " + creator->name())
    }

    AGZ_HIERARCHY_WRAP(
        "in creating object with factory: " + this->factory<T>().name())
}

AGZ_TRACER_FACTORY_END
