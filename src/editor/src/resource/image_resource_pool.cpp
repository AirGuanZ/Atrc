#include <QApplication>

#include <agz/editor/editor.h>
#include <agz/editor/resource/pool/image_resource_pool.h>
#include <agz/tracer/utility/logger.h>

AGZ_EDITOR_BEGIN

namespace
{
    constexpr int TEXTURE2D_THUMBNAIL_RENDER_SIZE = 96;
}

template<typename TracerObject>
ImageResourcePool<TracerObject>::ImageResourcePool(
    ObjectContext &obj_ctx, Editor *editor, const QString &default_type)
    : obj_ctx_(obj_ctx), editor_(editor), default_type_(default_type)
{
    ui_ = new ImageResourcePoolWidget;
    layout_ = new FlowLayout(ui_->scroll_area_widget);

    layout_->setContentsMargins(0, 0, 0, 0);

    icon_font_ = QApplication::font();
    icon_font_.setPointSize(9);

    connect(ui_->create, &QPushButton::clicked, [=]
    {
        bool ok = false;
        const QString name = to_valid_name(QInputDialog::getText(
            ui_, "Name", "Enter resource name", QLineEdit::Normal, {}, &ok));
        if(!ok)
            return;

        AGZ_INFO("create new '{}' with name: {}",
                 typeid(TracerObject).name(), name.toStdString());

        auto panel = newBox<ResourcePanel<TracerObject>>(obj_ctx_, default_type);
        add_resource(name, std::move(panel));
    });

    connect(ui_->remove, &QPushButton::clicked, [=]
    {
        if(!selected_rsc_)
            return;

        auto icon = selected_rsc_->icon;
        const QString name = selected_rsc_->name;
        selected_rsc_ = nullptr;
        name2rsc_.erase(name);

        AGZ_INFO("remove '{}' with name: {}",
                 typeid(TracerObject).name(), name.toStdString());

        delete icon;
    });

    connect(ui_->duplicate, &QPushButton::clicked, [=]
    {
        if(!selected_rsc_)
            return;

        bool ok = false;
        const QString name = to_valid_name(QInputDialog::getText(
            ui_, "Name", "Enter resource name", QLineEdit::Normal, {}, &ok));
        if(!ok)
            return;
        
        AGZ_INFO("duplicate '{}' : {} -> {}",
            typeid(TracerObject).name(),
            selected_rsc_->name.toStdString(), name.toStdString());

        auto panel = selected_rsc_->rsc->clone_panel();
        add_resource(name, Box<ResourcePanel<TracerObject>>(panel));
    });

    connect(ui_->edit, &QPushButton::clicked, [=]
    {
        if(!selected_rsc_)
            return;

        show_edit_panel(selected_rsc_->rsc->get_panel(), true);
    });

    connect(ui_->rename, &QPushButton::clicked, [=]
    {
        if(!selected_rsc_)
            return;

        bool ok = false;
        const QString name = to_valid_name(QInputDialog::getText(
            ui_, "Name", "Enter resource name", QLineEdit::Normal, {}, &ok));
        if(!ok)
            return;

        selected_rsc_->name = name;
        selected_rsc_->icon->set_text(name);
        selected_rsc_->rsc->set_name(name);
    });
}

template<typename TracerObject>
Box<ResourceReference<TracerObject>> ImageResourcePool<TracerObject>::select_resource()
{
    if(selected_rsc_)
        return selected_rsc_->rsc->create_reference();
    return nullptr;
}

template<typename TracerObject>
ResourceInPool<TracerObject> *ImageResourcePool<TracerObject>::add_resource(
    const QString &name, Box<ResourcePanel<TracerObject>> panel)
{
    auto *raw_panel = panel.get();
    auto rsc = newBox<ResourceInPool<TracerObject>>(name, std::move(panel));

    auto raw_rsc = rsc.get();
    editor_->add_to_resource_panel(raw_panel);

    ImageTextIcon *icon = new ImageTextIcon(
        ui_->scroll_area_widget, icon_font_, TEXTURE2D_THUMBNAIL_RENDER_SIZE);
    icon->set_text(name);
    icon->set_image(raw_panel->get_thumbnail(
        TEXTURE2D_THUMBNAIL_RENDER_SIZE, TEXTURE2D_THUMBNAIL_RENDER_SIZE));

    auto record = newBox<Record>();
    auto raw_record = record.get();
    record->name = name;
    record->rsc  = std::move(rsc);
    record->icon = icon;
    name2rsc_[name] = std::move(record);

    connect(icon, &ImageTextIcon::clicked, [=]
    {
        if(!raw_record->icon->is_selected())
        {
            set_selected_rsc(raw_record);
            show_edit_panel(raw_record->rsc->get_panel(), false);
        }
        else
            set_selected_rsc(nullptr);
    });

    raw_rsc->set_dirty_callback([=]
    {
        icon->set_image(raw_panel->get_thumbnail(
            TEXTURE2D_THUMBNAIL_RENDER_SIZE, TEXTURE2D_THUMBNAIL_RENDER_SIZE));
    });

    layout_->addWidget(icon);

    return raw_rsc;
}

template<typename TracerObject>
void ImageResourcePool<TracerObject>::save_asset(AssetSaver &saver) const
{
    saver.write(uint32_t(name2rsc_.size()));
    for(auto &p : name2rsc_)
        p.second->rsc->save_asset(saver);
}

template<typename TracerObject>
void ImageResourcePool<TracerObject>::load_asset(AssetLoader &loader)
{
    const uint32_t count = loader.read<uint32_t>();
    for(uint32_t i = 0; i < count; ++i)
    {
        auto rsc = newBox<ResourceInPool<TracerObject>>(
            "", newBox<ResourcePanel<TracerObject>>(obj_ctx_, default_type_));
        rsc->load_asset(*this, loader);
        const QString name = rsc->get_name();

        auto raw_rsc = rsc.get();
        auto raw_panel = rsc->get_panel();
        editor_->add_to_resource_panel(raw_panel);

        ImageTextIcon *icon = new ImageTextIcon(
            ui_->scroll_area_widget, icon_font_, TEXTURE2D_THUMBNAIL_RENDER_SIZE);
        icon->set_text(name);
        icon->set_image(raw_panel->get_thumbnail(
            TEXTURE2D_THUMBNAIL_RENDER_SIZE, TEXTURE2D_THUMBNAIL_RENDER_SIZE));

        auto record = newBox<Record>();
        auto raw_record = record.get();
        record->name = name;
        record->rsc  = std::move(rsc);
        record->icon = icon;
        name2rsc_[name] = std::move(record);

        connect(icon, &ImageTextIcon::clicked, [=]
        {
            if(!raw_record->icon->is_selected())
            {
                set_selected_rsc(raw_record);
                show_edit_panel(raw_record->rsc->get_panel(), false);
            }
            else
                set_selected_rsc(nullptr);
        });

        raw_rsc->set_dirty_callback([=]
        {
            icon->set_image(raw_panel->get_thumbnail(
                TEXTURE2D_THUMBNAIL_RENDER_SIZE, TEXTURE2D_THUMBNAIL_RENDER_SIZE));
        });

        layout_->addWidget(icon);
    }
}

template<typename TracerObject>
void ImageResourcePool<TracerObject>::to_config(
    tracer::ConfigGroup &scene_grp, JSONExportContext &ctx) const
{
    for(auto &p : name2rsc_)
    {
        auto rsc = p.second->rsc.get();
        auto ref_name = rsc->get_config_ref_name();

        tracer::ConfigGroup *grp = &scene_grp;
        for(size_t i = 0; i < ref_name->size() - 1; ++i)
        {
            auto sub_grp = grp->find_child_group(ref_name->at_str(i));
            if(sub_grp)
            {
                grp = sub_grp;
                continue;
            }

            auto new_grp = newRC<tracer::ConfigGroup>();
            grp->insert_child(ref_name->at_str(i), new_grp);
            grp = new_grp.get();
        }

        auto rsc_grp = rsc->to_config(ctx);
        assert(rsc_grp);
        grp->insert_child(ref_name->at_str(ref_name->size() - 1), rsc_grp);
    }
}

template<typename TracerObject>
ResourceInPool<TracerObject> *ImageResourcePool<TracerObject>::name_to_rsc(
    const QString &name)
{
    auto it = name2rsc_.find(name);
    return it == name2rsc_.end() ? nullptr : it->second->rsc.get();
}

template<typename TracerObject>
bool ImageResourcePool<TracerObject>::is_valid_name(const QString &name) const
{
    return name2rsc_.find(name) == name2rsc_.end();
}

template<typename TracerObject>
QString ImageResourcePool<TracerObject>::to_valid_name(const QString &name) const
{
    if(name.isEmpty())
        return to_valid_name("auto");

    if(is_valid_name(name))
        return name;

    for(int index = 0;; ++index)
    {
        QString ret = QString("%1 (%2)").arg(name).arg(index);
        if(is_valid_name(ret))
            return ret;
    }
}

template<typename TracerObject>
void ImageResourcePool<TracerObject>::show_edit_panel(
    ResourcePanel<TracerObject> *rsc, bool display_rsc_panel)
{
    editor_->show_resource_panel(rsc, display_rsc_panel);
}

template<typename TracerObject>
QWidget *ImageResourcePool<TracerObject>::get_widget()
{
    return ui_;
}

template<typename TracerObject>
void ImageResourcePool<TracerObject>::set_selected_rsc(Record *record)
{
    if(selected_rsc_)
        selected_rsc_->icon->set_selected(false);

    selected_rsc_ = record;
    if(selected_rsc_)
        selected_rsc_->icon->set_selected(true);
}

template class ImageResourcePool<tracer::Material>;
template class ImageResourcePool<tracer::Texture2D>;
template class ImageResourcePool<tracer::Texture3D>;

AGZ_EDITOR_END
