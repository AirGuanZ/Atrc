#pragma once

#include <QScrollArea>
#include <QVBoxLayout>

#include <agz/editor/resource.h>

AGZ_EDITOR_BEGIN

using Texture2DWidget        = ResourceWidget       <tracer::Texture2D>;
using Texture2DWidgetCreator = ResourceWidgetCreator<tracer::Texture2D>;
using Texture2DWidgetFactory = ResourceWidgetFactory<tracer::Texture2D>;
using Texture2DPanel         = ResourcePanel        <tracer::Texture2D>;
using Texture2DReference     = ResourceReference    <tracer::Texture2D>;
using Texture2DInPool        = ResourceInPool       <tracer::Texture2D>;
using Texture2DSlot          = ResourceSlot         <tracer::Texture2D>;

class Editor;

class Texture2DPool : public QObject, public ResourcePool<tracer::Texture2D>
{
public:

    Texture2DPool(const Texture2DWidgetFactory &factory, Editor *editor);

    std::unique_ptr<ResourceReference<tracer::Texture2D>> select_resource() override;

    ResourceInPool<tracer::Texture2D> *add_resource(QStringView name, std::unique_ptr<ResourcePanel<tracer::Texture2D>> panel) override;

    bool is_valid_name(QStringView name) const override;

    void show_edit_panel(ResourcePanel<tracer::Texture2D> *rsc) override;

    QWidget *get_widget();

private:

    const Texture2DWidgetFactory &factory_;

    Editor *editor_;

    std::map<QString, std::unique_ptr<Texture2DInPool>, std::less<>> name2rsc_;

    QScrollArea *scroll_area_        = nullptr;
    QWidget     *scroll_area_widget_ = nullptr;
    QVBoxLayout *layout_             = nullptr;
};

void init_texture2d_factory(Texture2DWidgetFactory &factory);

AGZ_EDITOR_END
