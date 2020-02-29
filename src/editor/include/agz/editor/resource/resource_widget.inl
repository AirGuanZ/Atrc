#pragma once

AGZ_EDITOR_BEGIN

inline ResourceObjectException::ResourceObjectException(const QString &msg)
    : msg_(msg)
{

}

inline char const *ResourceObjectException::what() const
{
    return msg_.toStdString().c_str();
}

inline const QString &ResourceObjectException::msg() const noexcept
{
    return msg_;
}

template<typename TracerObject>
void ResourceWidget<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourceWidget<TracerObject>::get_tracer_object()
{
    return tracer_object_;
}

template<typename TracerObject>
std::unique_ptr<ResourceThumbnailProvider> ResourceWidget<TracerObject>::get_thumbnail(int width, int height) const
{
    return std::make_unique<EmptyResourceThumbnailProvider>(width, height);
}

template<typename TracerObject>
void ResourceWidget<TracerObject>::set_dirty_flag()
{
    update_tracer_object_impl();
    if(dirty_callback_)
        dirty_callback_();
}

template<typename TracerObject>
void ResourceWidgetFactory<TracerObject>::add_creator(std::unique_ptr<Creator> creator)
{
    assert(creator);

    QString name = creator->name();
    if(name2creator_.find(name) != name2creator_.end())
        throw ResourceObjectException("repeated resource widget creator name: " + name);
    type_names_.push_back(name);

    name2creator_.insert(std::make_pair(std::move(name), std::move(creator)));
}

template<typename TracerObject>
const QStringList &ResourceWidgetFactory<TracerObject>::get_type_names() const noexcept
{
    return type_names_;
}

template<typename TracerObject>
typename ResourceWidgetFactory<TracerObject>::Widget *
ResourceWidgetFactory<TracerObject>::create_widget(const QString &name, ObjectContext &obj_ctx) const
{
    const auto it = name2creator_.find(name);
    if(it == name2creator_.end())
        throw ResourceObjectException("unknown resource widget creator name: " + name);
    return it->second->create_widget(obj_ctx);
}

AGZ_EDITOR_END
