#pragma once

#include <QInputDialog>
#include <QPushButton>

AGZ_EDITOR_BEGIN

template<typename TracerObject>
ResourceSlot<TracerObject>::ResourceSlot(
    ObjectContext &obj_ctx, const QString &default_type)
    : ResourceSlot(obj_ctx, default_type, nullptr,
                   new ResourcePanel<TracerObject>(obj_ctx, default_type))
{

}

template<typename TracerObject>
ResourceSlot<TracerObject>::ResourceSlot(
    ObjectContext &obj_ctx, const QString &default_type,
    ResourceWidget<TracerObject> *widget, const QString &widget_type)
    : ResourceSlot(
        obj_ctx, default_type, nullptr,
        new ResourcePanel<TracerObject>(obj_ctx, widget, widget_type))
{

}

template<typename TracerObject>
ResourceSlot<TracerObject>::~ResourceSlot()
{
    if(owned_panel_)
        delete owned_panel_;
}

template<typename TracerObject>
void ResourceSlot<TracerObject>::set_reference(
    Box<ResourceReference<TracerObject>> reference)
{
    if(reference_)
        reference_.reset();
    else if(owned_panel_)
    {
        delete owned_panel_;
        owned_panel_ = nullptr;
    }

    reference_ = std::move(reference);
    reference_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
    });
    reference_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
    });
    reference_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
    });
    reference_->set_removed_callback([=] { reference_removed_callback_impl(); });

    reference_widget_   ->show();
    owned_button_widget_->hide();

    reference_layout_->insertWidget(0, reference_->get_name_widget());

    if(dirty_callback_)
        dirty_callback_();

    set_geometry_vertices_dirty();
    set_entity_transform_dirty();
}

template<typename TracerObject>
void ResourceSlot<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
RC<TracerObject> ResourceSlot<TracerObject>::get_tracer_object()
{
    if(reference_)
        return reference_->get_tracer_object();
    assert(owned_panel_);
    return owned_panel_->get_tracer_object();
}

template<typename TracerObject>
Box<ResourceThumbnailProvider> ResourceSlot<TracerObject>::get_thumbnail(int width, int height)
{
    if(reference_)
        return reference_->get_resource()->get_thumbnail(width, height);
    assert(owned_panel_);
    return owned_panel_->get_thumbnail(width, height);
}

template<typename TracerObject>
ResourceSlot<TracerObject> *ResourceSlot<TracerObject>::clone() const
{
    Box<ResourceReference<TracerObject>> reference;
    if(reference_)
        reference = reference_->get_resource()->create_reference();

    ResourcePanel<TracerObject> *owned_panel = nullptr;
    if(owned_panel_)
        owned_panel = owned_panel_->clone();

    return new ResourceSlot<TracerObject>(
        obj_ctx_, default_panel_type_, std::move(reference), owned_panel);
}

template<typename TracerObject>
void ResourceSlot<TracerObject>::save_asset(AssetSaver &saver) const
{
    const uint8_t is_reference =
        (saver.is_rsc_pool_enabled<TracerObject>() && reference_ != nullptr) ? 1 : 0;
    saver.write(is_reference);

    if(is_reference)
    {
        assert(!owned_panel_);
        saver.write_string(reference_->get_resource()->get_name());
    }
    else
    {
        if(owned_panel_)
        {
            assert(!reference_);
            owned_panel_->save_asset(saver);
        }
        else
        {
            assert(reference_);
            reference_->get_resource()->get_panel()->save_asset(saver);
        }
    }
}

template<typename TracerObject>
void ResourceSlot<TracerObject>::load_asset(AssetLoader &loader)
{
    assert(!reference_ && owned_panel_);

    const uint8_t is_reference = loader.read<uint8_t>();
    if(is_reference)
    {
        const QString rsc_name = loader.read_string();
        loader.add_delayed_opr([rsc_name, &loader, this]
        {
            const QString mapped_rsc_name = loader.rsc_name_map<TracerObject>(rsc_name);
            auto rsc = obj_ctx_.pool<TracerObject>()->name_to_rsc(mapped_rsc_name);
            if(rsc)
                set_reference(rsc->create_reference());
        });
    }
    else
        owned_panel_->load_asset(loader);
}

template<typename TracerObject>
RC<tracer::ConfigNode> ResourceSlot<TracerObject>::to_config(JSONExportContext &ctx) const
{
    if(reference_)
        return reference_->to_config();
    assert(owned_panel_);
    return owned_panel_->to_config(ctx);
}

template<typename TracerObject>
std::vector<EntityInterface::Vertex> ResourceSlot<TracerObject>::get_vertices() const
{
    if(reference_)
        return reference_->get_vertices();
    return owned_panel_->get_vertices();
}

template<typename TracerObject>
DirectTransform ResourceSlot<TracerObject>::get_transform() const
{
    if(reference_)
        return reference_->get_transform();
    return owned_panel_->get_transform();
}

template<typename TracerObject>
void ResourceSlot<TracerObject>::set_transform(const DirectTransform &transform)
{
    if(reference_)
        reference_->set_transform(transform);
    else
        owned_panel_->set_transform(transform);
}

template<typename TracerObject>
ResourceSlot<TracerObject>::ResourceSlot(
    ObjectContext &obj_ctx, const QString &default_type,
    Box<ResourceReference<TracerObject>> reference,
    ResourcePanel<TracerObject> *owned_panel)
    : obj_ctx_(obj_ctx), default_panel_type_(default_type)
{
    auto pool = obj_ctx_.pool<TracerObject>();
    const bool has_pool = pool != nullptr;

    owned_panel_ = owned_panel;
    reference_ = std::move(reference);

    if(owned_panel_)
    {
        owned_panel_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });
        owned_panel_->set_geometry_vertices_dirty_callback([=]
        {
            set_geometry_vertices_dirty();
        });
        owned_panel_->set_entity_transform_dirty_callback([=]
        {
            set_entity_transform_dirty();
        });
    }
    else
    {
        assert(reference_);
        reference_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });
        reference_->set_geometry_vertices_dirty_callback([=]
        {
            set_geometry_vertices_dirty();
        });
        reference_->set_entity_transform_dirty_callback([=]
        {
            set_entity_transform_dirty();
        });
    }

    if(has_pool)
    {
        reference_widget_    = new QWidget(this);
        owned_button_widget_ = new QWidget(this);

        reference_layout_                    = new QVBoxLayout(reference_widget_);

        QWidget     *reference_button_widget = new QWidget(reference_widget_);
        QHBoxLayout *reference_button_layout = new QHBoxLayout(reference_button_widget);
        QHBoxLayout *owned_button_layout     = new QHBoxLayout(owned_button_widget_);

        reference_widget_->setContentsMargins(0, 0, 0, 0);
        reference_button_widget->setContentsMargins(0, 0, 0, 0);
        owned_button_widget_->setContentsMargins(0, 0, 0, 0);

        if(reference_)
            reference_layout_->addWidget(reference_->get_name_widget());
        reference_layout_->addWidget(reference_button_widget);

        reference_layout_->setContentsMargins(0, 0, 0, 0);
        reference_button_layout->setContentsMargins(0, 0, 0, 0);
        owned_button_layout->setContentsMargins(0, 0, 0, 0);
        
        QPushButton *create         = new QPushButton("Create", reference_button_widget);
        QPushButton *select_in_pool = new QPushButton("Select", reference_button_widget);
        QPushButton *edit_rsc       = new QPushButton("Edit",   reference_button_widget);
        QPushButton *break_link     = new QPushButton("Unlink", reference_button_widget);

        QPushButton *owned_select_in_pool = new QPushButton("Select",      owned_button_widget_);
        QPushButton *add_to_pool          = new QPushButton("Add to Pool", owned_button_widget_);

        // delete the reference and create a new panel
        signal_to_callback_.connect_callback(create, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);

            reference_.reset();

            owned_panel_ = new ResourcePanel<TracerObject>(
                obj_ctx_, default_panel_type_);
            owned_panel_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            owned_panel_->set_geometry_vertices_dirty_callback([=]
            {
                set_geometry_vertices_dirty();
            });
            owned_panel_->set_entity_transform_dirty_callback([=]
            {
                set_entity_transform_dirty();
            });
            layout_->addWidget(owned_panel_);

            reference_widget_   ->hide();
            owned_button_widget_->show();

            if(dirty_callback_)
                dirty_callback_();

            set_geometry_vertices_dirty();
            set_entity_transform_dirty();
        });

        // delete current reference and select a new one from pool
        signal_to_callback_.connect_callback(
            select_in_pool, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);

            auto new_ref = obj_ctx_.pool<TracerObject>()->select_resource();
            if(!new_ref)
                return;

            set_reference(std::move(new_ref));
        });

        // edit the referenced resource
        signal_to_callback_.connect_callback(
            edit_rsc, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);
            obj_ctx_.pool<TracerObject>()->show_edit_panel(
                reference_->get_resource()->get_panel(), true);
        });

        // clone the referenced resource as my own panel and destroy the reference
        signal_to_callback_.connect_callback(
            break_link, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);

            owned_panel_ = reference_->get_resource()->clone_panel();
            owned_panel_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            owned_panel_->set_geometry_vertices_dirty_callback([=]
            {
                set_geometry_vertices_dirty();
            });
            owned_panel_->set_entity_transform_dirty_callback([=]
            {
                set_entity_transform_dirty();
            });
            layout_->addWidget(owned_panel_);

            reference_.reset();

            reference_widget_   ->hide();
            owned_button_widget_->show();

            if(dirty_callback_)
                dirty_callback_();

            set_geometry_vertices_dirty();
            set_entity_transform_dirty();
        });

        // delete current panel and select a new one from pool
        signal_to_callback_.connect_callback(
            owned_select_in_pool, &QPushButton::clicked, [=]
        {
            assert(!reference_ && owned_panel_);

            auto new_ref = obj_ctx_.pool<TracerObject>()->select_resource();
            if(!new_ref)
                return;

            set_reference(std::move(new_ref));
        });

        // add current panel to pool and hold a reference to it
        signal_to_callback_.connect_callback(
            add_to_pool, &QPushButton::clicked, [=]
        {
            assert(!reference_ && owned_panel_);

            bool ok = false;
            const QString name = obj_ctx_.pool<TracerObject>()
                ->to_valid_name(QInputDialog::getText(
                    this, "Name", "Enter resource name", QLineEdit::Normal, {}, &ok));
            if(!ok)
                return;

            auto rsc = obj_ctx_.pool<TracerObject>()->add_resource(
                name, Box<ResourcePanel<TracerObject>>(owned_panel_));
            owned_panel_ = nullptr;

            set_reference(rsc->create_reference());
        });

        reference_button_layout->addWidget(create);
        reference_button_layout->addWidget(select_in_pool);
        reference_button_layout->addWidget(edit_rsc);
        reference_button_layout->addWidget(break_link);

        owned_button_layout->addWidget(owned_select_in_pool);
        owned_button_layout->addWidget(add_to_pool);
    }

    layout_ = new QVBoxLayout(this);
    if(has_pool)
    {
        layout_->addWidget(reference_widget_);
        layout_->addWidget(owned_button_widget_);
        if(owned_panel_)
            reference_widget_->hide();
        else
            owned_button_widget_->hide();
    }
    layout_->addWidget(owned_panel_);
}

template<typename TracerObject>
void ResourceSlot<TracerObject>::reference_removed_callback_impl()
{
    owned_panel_ = reference_->get_resource()->clone_panel();
    owned_panel_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
    });
    owned_panel_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
    });
    owned_panel_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
    });
    layout_->addWidget(owned_panel_);

    (void)reference_.release();

    reference_widget_->hide();
    owned_button_widget_->show();

    if(dirty_callback_)
        dirty_callback_();

    set_geometry_vertices_dirty();
    set_entity_transform_dirty();
}

AGZ_EDITOR_END
