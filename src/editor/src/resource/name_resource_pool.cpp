#include <agz/editor/editor.h>
#include <agz/editor/resource/name_resource_pool.h>

AGZ_EDITOR_BEGIN

namespace
{
    constexpr int WIDGET_ITEM_HEIGHT = 35;
}

template<typename TracerObject>
NameResourcePool<TracerObject>::NameResourcePool(
    ObjectContext &obj_ctx, Editor *editor, const QString &default_type)
    : obj_ctx_(obj_ctx), editor_(editor)
{
    widget_ = new NameResourcePoolWidget;

    connect(widget_->create, &QPushButton::clicked, [=]
    {
        const QString name = QInputDialog::getText(
            widget_, "Name", "Enter resource name");
        if(name.isEmpty())
            return;

        if(!is_valid_name(name))
        {
            show_invalid_name_mbox(name);
            return;
        }

        AGZ_INFO("create new '{}' with name: {}",
                 typeid(TracerObject).name(), name.toStdString());

        auto panel = std::make_unique<ResourcePanel<TracerObject>>(obj_ctx_, default_type);
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

        const QString new_name = QInputDialog::getText(
            widget_, "Name", "Enter resource name");
        if(new_name.isEmpty())
            return;

        if(!is_valid_name(new_name))
        {
            show_invalid_name_mbox(new_name);
            return;
        }

        AGZ_INFO("duplicate '{}' : {} -> {}",
            typeid(TracerObject).name(), it->second->name.toStdString(), new_name.toStdString());

        auto panel = it->second->rsc->clone_panel();
        add_resource(new_name, std::unique_ptr<ResourcePanel<TracerObject>>(panel));
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

        const QString new_name = QInputDialog::getText(
            widget_, "Name", "Enter resource name");
        if(new_name.isEmpty())
            return;

        if(!is_valid_name(new_name))
        {
            show_invalid_name_mbox(new_name);
            return;
        }

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
std::unique_ptr<ResourceReference<TracerObject>>
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
    const QString &name, std::unique_ptr<ResourcePanel<TracerObject>> panel)
{
    auto *raw_panel = panel.get();
    auto rsc = std::make_unique<ResourceInPool<TracerObject>>(name, std::move(panel));

    auto raw_rsc = rsc.get();
    editor_->add_to_resource_panel(raw_panel);

    auto record = std::make_unique<Record>();
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
bool NameResourcePool<TracerObject>::is_valid_name(const QString &name) const
{
    return name2record_.find(name) == name2record_.end();
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

AGZ_EDITOR_END
