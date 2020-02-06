#include <agz/editor/editor.h>
#include <agz/editor/texture2d/texture2d.h>

AGZ_EDITOR_BEGIN

Texture2DPool::Texture2DPool(const Texture2DWidgetFactory &factory, Editor *editor)
    : factory_(factory), editor_(editor)
{
    scroll_area_        = new QScrollArea;
    scroll_area_widget_ = new QWidget(scroll_area_);
    layout_             = new QVBoxLayout(scroll_area_widget_);
    scroll_area_->setWidget(scroll_area_widget_);
}

std::unique_ptr<ResourceReference<tracer::Texture2D>> Texture2DPool::select_resource()
{
    return nullptr;
}

ResourceInPool<tracer::Texture2D> *Texture2DPool::add_resource(QStringView name, std::unique_ptr<ResourcePanel<tracer::Texture2D>> panel)
{
    auto *raw_panel = panel.get();
    std::unique_ptr<Texture2DInPool> rsc = std::make_unique<Texture2DInPool>(std::move(panel));
    auto raw_rsc = rsc.get();
    name2rsc_[name.toString()] = std::move(rsc);
    editor_->add_to_resource_panel(raw_panel);
    return raw_rsc;
}

bool Texture2DPool::is_valid_name(QStringView name) const
{
    return name2rsc_.find(name) == name2rsc_.end();
}

void Texture2DPool::show_edit_panel(ResourcePanel<tracer::Texture2D> *rsc)
{
    editor_->show_resource_panel(rsc);
}

QWidget *Texture2DPool::get_widget()
{
    return scroll_area_;
}

AGZ_EDITOR_END
