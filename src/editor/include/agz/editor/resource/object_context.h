#pragma once

#include <agz/editor/resource/resource_pool.h>

AGZ_EDITOR_BEGIN

class Editor;

class ObjectContext
{
    template<typename...TracerObjects>
    using FactoryTuple = std::tuple<ResourceWidgetFactory<TracerObjects>...>;

    template<typename...TracerObjects>
    using PoolTuple = std::tuple<std::unique_ptr<ResourcePool<TracerObjects>>...>;

    template <typename T, typename Tuple>
    struct has_type;

    template <typename T, typename... Us>
    struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

    FactoryTuple<
        tracer::Entity,
        tracer::EnvirLight,
        tracer::Geometry,
        tracer::Material,
        tracer::Medium,
        tracer::Texture2D,
        tracer::Texture3D> factorys_;

    PoolTuple<
        tracer::Geometry,
        tracer::Material,
        tracer::Medium,
        tracer::Texture2D,
        tracer::Texture3D> pools_;

public:

    explicit ObjectContext(Editor *editor);

    template<typename TracerObject>
    const ResourceWidgetFactory<TracerObject> &factory() const noexcept
    {
        return std::get<ResourceWidgetFactory<TracerObject>>(factorys_);
    }

    template<typename TracerObject>
    const ResourcePool<TracerObject> *pool() const noexcept
    {
        if constexpr(!has_type<std::unique_ptr<ResourcePool<TracerObject>>, decltype(pools_)>::value)
            return nullptr;
        else
            return std::get<std::unique_ptr<ResourcePool<TracerObject>>>(pools_).get();
    }

    template<typename TracerObject>
    ResourceWidgetFactory<TracerObject> &factory() noexcept
    {
        return std::get<ResourceWidgetFactory<TracerObject>>(factorys_);
    }

    template<typename TracerObject>
    ResourcePool<TracerObject> *pool() noexcept
    {
        if constexpr(!has_type<std::unique_ptr<ResourcePool<TracerObject>>, decltype(pools_)>::value)
            return nullptr;
        else
            return std::get<std::unique_ptr<ResourcePool<TracerObject>>>(pools_).get();
    }
};

AGZ_EDITOR_END
