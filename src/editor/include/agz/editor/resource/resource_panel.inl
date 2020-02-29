#pragma once

AGZ_EDITOR_BEGIN

template<typename TracerObject>
ResourcePanel<TracerObject>::ResourcePanel(ObjectContext &obj_ctx, const QString &default_type)
    : obj_ctx_(obj_ctx)
{
    type_selector_ = new ComboBoxWithoutWheelFocus(this);
    type_selector_->addItems(obj_ctx.factory<TracerObject>().get_type_names());
    type_selector_->setCurrentText(default_type);

    auto change_type = [=](const QString &new_type)
    {
        if(rsc_widget_)
            delete rsc_widget_;

        rsc_widget_ = obj_ctx_.factory<TracerObject>().create_widget(new_type, obj_ctx_);
        rsc_widget_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });
        rsc_widget_->set_geometry_vertices_dirty_callback([=]
        {
            set_geometry_vertices_dirty();
        });
        rsc_widget_->set_entity_transform_dirty_callback([=]
        {
            set_entity_transform_dirty();
        });

        layout_->addWidget(rsc_widget_);

        if(dirty_callback_)
            dirty_callback_();

        set_geometry_vertices_dirty();
        set_entity_transform_dirty();
    };

    signal_to_callback_.connect_callback(
        type_selector_, &ComboBoxWithoutWheelFocus::currentTextChanged, change_type);

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(type_selector_);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);

    change_type(default_type);
}

template<typename TracerObject>
ResourcePanel<TracerObject>::ResourcePanel(
    ObjectContext &obj_ctx,
    ResourceWidget<TracerObject> *rsc_widget, const QString &current_type_name)
    : obj_ctx_(obj_ctx)
{
    type_selector_ = new ComboBoxWithoutWheelFocus(this);
    type_selector_->addItems(obj_ctx.factory<TracerObject>().get_type_names());
    type_selector_->setCurrentText(current_type_name);

    signal_to_callback_.connect_callback(
        type_selector_, &ComboBoxWithoutWheelFocus::currentTextChanged, [=](const QString &new_type)
    {
        on_change_selected_type();
    });

    rsc_widget_ = rsc_widget;
    rsc_widget_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
    });
    rsc_widget_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
    });
    rsc_widget_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
    });

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(type_selector_);
    layout_->addWidget(rsc_widget_);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);
}

template<typename TracerObject>
void ResourcePanel<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourcePanel<TracerObject>::get_tracer_object()
{
    return rsc_widget_->get_tracer_object();
}

template<typename TracerObject>
ResourcePanel<TracerObject> *ResourcePanel<TracerObject>::clone() const
{
    return new ResourcePanel(obj_ctx_, rsc_widget_->clone(), type_selector_->currentText());
}

template<typename TracerObject>
std::unique_ptr<ResourceThumbnailProvider> ResourcePanel<TracerObject>::get_thumbnail(int width, int height) const
{
    return rsc_widget_->get_thumbnail(width, height);
}

template<typename TracerObject>
void ResourcePanel<TracerObject>::save_asset(AssetSaver &saver) const
{
    saver.write_string(type_selector_->currentText());
    rsc_widget_->save_asset(saver);
}

template<typename TracerObject>
void ResourcePanel<TracerObject>::load_asset(AssetLoader &loader)
{
    const QString type = loader.read_string();
    type_selector_->setCurrentText(type);
    rsc_widget_->load_asset(loader);
}

template<typename TracerObject>
std::vector<EntityInterface::Vertex> ResourcePanel<TracerObject>::get_vertices() const
{
    return rsc_widget_->get_vertices();
}

template<typename TracerObject>
DirectTransform ResourcePanel<TracerObject>::get_transform() const
{
    return rsc_widget_->get_transform();
}

template<typename TracerObject>
void ResourcePanel<TracerObject>::set_transform(const DirectTransform &transform)
{
    rsc_widget_->set_transform(transform);
}

template<typename TracerObject>
void ResourcePanel<TracerObject>::on_change_selected_type()
{
    if(rsc_widget_)
        delete rsc_widget_;

    const QString new_type = type_selector_->currentText();

    rsc_widget_ = obj_ctx_.factory<TracerObject>().create_widget(new_type, obj_ctx_);
    rsc_widget_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
    });
    rsc_widget_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
    });
    rsc_widget_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
    });

    layout_->addWidget(rsc_widget_);

    if(dirty_callback_)
        dirty_callback_();

    set_geometry_vertices_dirty();
    set_entity_transform_dirty();
}

AGZ_EDITOR_END
