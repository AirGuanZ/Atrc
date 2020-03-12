#pragma once

#include <QObject>

#include <agz/editor/utility/direct_transform3d.h>

AGZ_EDITOR_BEGIN

class EntityInterface
{
public:

    struct Vertex
    {
        Vec3 pos;
        Vec3 nor;
    };

    virtual ~EntityInterface() = default;

    void set_geometry_vertices_dirty_callback(std::function<void()> callback)
    {
        geometry_callback_ = std::move(callback);
    }

    void set_entity_transform_dirty_callback(std::function<void()> callback)
    {
        transform_callback_ = std::move(callback);
    }

    virtual void set_transform(const DirectTransform &transform)
    {
        throw std::runtime_error(
            "EntityInterface::set_transform is unimplemented");
    }

    virtual DirectTransform get_transform() const
    {
        throw std::runtime_error(
            "EntityInterface::get_transform is unimplemented");
    }

    virtual std::vector<Vertex> get_vertices() const
    {
        throw std::runtime_error(
            "EntityInterface::get_vertices is unimplemented");
    }

protected:

    void set_geometry_vertices_dirty()
    {
        if(geometry_callback_)
            geometry_callback_();
    }

    void set_entity_transform_dirty()
    {
        if(transform_callback_)
            transform_callback_();
    }

private:

    std::function<void()> geometry_callback_;
    std::function<void()> transform_callback_;
};

AGZ_EDITOR_END
