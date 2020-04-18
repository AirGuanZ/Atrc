#include <agz/editor/editor.h>
#include <agz/editor/resource/pool/name_resource_pool.h>
#include <agz/tracer/utility/logger.h>

AGZ_EDITOR_BEGIN

namespace
{
    constexpr int WIDGET_ITEM_HEIGHT = 35;
}

template<typename TracerObject>
NameResourcePool<TracerObject>::NameResourcePool(
    ObjectContext &obj_ctx, Editor *editor, const QString &default_type)
    : default_type_(default_type), obj_ctx_(obj_ctx), editor_(editor)
{
    widget_ = new NameResourcePoolWidget;

    connect(widget_->create, &QPushButton::clicked, [=]
    {
        bool ok = false;
        const QString name = to_valid_name(QInputDialog::getText(
            widget_, "Name", "Enter resource name",
            QLineEdit::Normal, {}, &ok));
        if(!ok)
            return;

        AGZ_INFO("create new '{}' with name: {}",
                 typeid(TracerObject).name(), name.toStdString());

        auto panel = newBox<ResourcePanel<TracerObject>>(obj_ctx_, default_type);
        add_resource(name, std::move(panel));
    });

    connect(widget_->remove, &QPushButton::clicked, [=]
    {
        auto selected = widget_->name_list->currentItem();
        if(!selected)
            return;

        const QString name = selected->text();
        const auto it = name2record_.find(name);
        assert(it != name2record_.end());

        AGZ_INFO("remove '{}' with name: {}",
                 typeid(TracerObject).name(), name.toStdString());

        name2record_.erase(it);
        delete widget_->name_list->takeItem(widget_->name_list->row(selected));
    });

    connect(widget_->duplicate, &QPushButton::clicked, [=]
    {
        auto selected = widget_->name_list->currentItem();
        if(!selected)
            return;

        const QString old_name = selected->text();
        const auto it = name2record_.find(old_name);
        assert(it != name2record_.end());

        bool ok = false;
        const QString new_name = to_valid_name(QInputDialog::getText(
            widget_, "Name", "Enter resource name",
            QLineEdit::Normal, {}, &ok));
        if(!ok)
            return;

        AGZ_INFO("duplicate '{}' : {} -> {}",
            typeid(TracerObject).name(), 
            it->second->name.toStdString(), new_name.toStdString());

        auto panel = it->second->rsc->clone_panel();
        add_resource(new_name, Box<ResourcePanel<TracerObject>>(panel));
    });

    connect(widget_->edit, &QPushButton::clicked, [=]
    {
        auto selected = widget_->name_list->currentItem();
        if(!selected)
            return;

        const QString old_name = selected->text();
        const auto it = name2record_.find(old_name);
        assert(it != name2record_.end());

        show_edit_panel(it->second->rsc->get_panel(), true);
    });

    connect(widget_->rename, &QPushButton::clicked, [=]
    {
        auto selected = widget_->name_list->currentItem();
        if(!selected)
            return;

        bool ok = false;
        const QString new_name = to_valid_name(QInputDialog::getText(
            widget_, "Name", "Enter resource name",
            QLineEdit::Normal, {}, &ok));
        if(!ok)
            return;

        auto it = name2record_.find(selected->text());
        assert(it != name2record_.end());
        it->second->name = new_name;
        it->second->rsc->set_name(new_name);
        selected->setText(new_name);

        auto rcd = std::move(it->second);
        name2record_.erase(it);
        name2record_[new_name] = std::move(rcd);
    });

    connect(widget_->name_list, &QListWidget::currentItemChanged,
        [=](QListWidgetItem *current, QListWidgetItem *previous)
    {
        if(!current)
            return;
        auto it = name2record_.find(current->text());
        assert(it != name2record_.end());
        show_edit_panel(it->second->rsc->get_panel(), false);
    });
}

template<typename TracerObject>
Box<ResourceReference<TracerObject>>
    NameResourcePool<TracerObject>::select_resource()
{
    auto selected = widget_->name_list->currentItem();
    if(!selected)
        return nullptr;

    const QString name = selected->text();
    const auto it = name2record_.find(name);
    return it != name2record_.end() ? it->second->rsc->create_reference() : nullptr;
}

template<typename TracerObject>
ResourceInPool<TracerObject> *NameResourcePool<TracerObject>::add_resource(
    const QString &name, Box<ResourcePanel<TracerObject>> panel)
{
    auto *raw_panel = panel.get();
    auto rsc = newBox<ResourceInPool<TracerObject>>(name, std::move(panel));

    auto raw_rsc = rsc.get();
    editor_->add_to_resource_panel(raw_panel);

    auto record = newBox<Record>();
    auto raw_record = record.get();
    record->name = name;
    record->rsc = std::move(rsc);
    name2record_[name] = std::move(record);

    widget_->name_list->addItem(name);

    auto new_item = widget_->name_list->findItems(name, Qt::MatchExactly).front();
    new_item->setSizeHint(QSize(new_item->sizeHint().width(), WIDGET_ITEM_HEIGHT));

    return raw_rsc;
}

template<typename TracerObject>
void NameResourcePool<TracerObject>::save_asset(AssetSaver &saver) const
{
    saver.write(uint32_t(name2record_.size()));
    for(auto &p : name2record_)
        p.second->rsc->save_asset(saver);
}

template<typename TracerObject>
void NameResourcePool<TracerObject>::load_asset(AssetLoader &loader)
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

        auto record = newBox<Record>();
        auto raw_record = record.get();
        record->name = name;
        record->rsc = std::move(rsc);
        name2record_[name] = std::move(record);

        widget_->name_list->addItem(name);

        auto new_item = widget_->name_list->findItems(name, Qt::MatchExactly).front();
        new_item->setSizeHint(QSize(new_item->sizeHint().width(), WIDGET_ITEM_HEIGHT));
    }
}

template<typename TracerObject>
void NameResourcePool<TracerObject>::to_config(
    tracer::ConfigGroup &scene_grp, JSONExportContext &ctx) const
{
    for(auto &p : name2record_)
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
ResourceInPool<TracerObject> *NameResourcePool<TracerObject>::name_to_rsc(
    const QString &name)
{
    auto it = name2record_.find(name);
    return it == name2record_.end() ? nullptr : it->second->rsc.get();
}

template<typename TracerObject>
bool NameResourcePool<TracerObject>::is_valid_name(const QString &name) const
{
    return !name.isEmpty() && name2record_.find(name) == name2record_.end();
}

template<typename TracerObject>
QString NameResourcePool<TracerObject>::to_valid_name(const QString &name) const
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
void NameResourcePool<TracerObject>::show_edit_panel(
    ResourcePanel<TracerObject> *rsc, bool display_rsc_panel)
{
    editor_->show_resource_panel(rsc, display_rsc_panel);
}

template<typename TracerObject>
QWidget *NameResourcePool<TracerObject>::get_widget()
{
    return widget_;
}

template class NameResourcePool<tracer::Geometry>;
template class NameResourcePool<tracer::Medium>;

AGZ_EDITOR_END
