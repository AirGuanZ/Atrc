#include <agz/editor/medium/heterogeneous.h>
#include <agz/editor/texture3d/range3d.h>
#include <agz/editor/ui/utility/collapsible.h>
#include <agz/tracer/create/medium.h>

AGZ_EDITOR_BEGIN

HeterogeneousWidget::HeterogeneousWidget(
    const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    transform_ = init_data.transform;
    density_   = init_data.density;
    albedo_    = init_data.albedo;
    g_         = init_data.g;

    max_scattering_count_ = new QSpinBox(this);
    max_scattering_count_->setRange(1, (std::numeric_limits<int>::max)());
    max_scattering_count_->setValue(init_data.max_scattering_count);

    if(!transform_)
        transform_ = new Transform3DSeqWidget;

    if(!density_)
        density_ = new Texture3DSlot(obj_ctx_, "Constant");

    if(!albedo_)
        albedo_ = new Texture3DSlot(obj_ctx_, "Constant");

    if(!g_)
    {
        auto tex = new Range3DWidget({ real(-0.99), real(0.99), 0 });
        g_ = new Texture3DSlot(obj_ctx_, "Range", tex, "Range");
    }

    connect(transform_, &Transform3DSeqWidget::change_transform, [=]
    {
        set_dirty_flag();
    });

    density_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    albedo_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    g_->set_dirty_callback([=]
    {
        set_dirty_flag();
    });

    connect(max_scattering_count_, qOverload<int>(&QSpinBox::valueChanged),
        [=](int)
    {
        set_dirty_flag();
    });

    Collapsible *trans_sec   = new Collapsible(this, "Transform");
    Collapsible *density_sec = new Collapsible(this, "Density");
    Collapsible *albedo_sec  = new Collapsible(this, "Albedo");
    Collapsible *g_sec       = new Collapsible(this, "g");

    trans_sec  ->set_content_widget(transform_);
    density_sec->set_content_widget(density_);
    albedo_sec ->set_content_widget(albedo_);
    g_sec      ->set_content_widget(g_);

    trans_sec  ->open();
    density_sec->open();
    albedo_sec ->open();
    g_sec      ->open();

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(trans_sec,   0, 0, 1, 2);
    layout->addWidget(density_sec, 1, 0, 1, 2);
    layout->addWidget(albedo_sec,  2, 0, 1, 2);
    layout->addWidget(g_sec,       3, 0, 1, 2);
    layout->addWidget(new QLabel("Max Scattering Count"), 4, 0, 1, 1);
    layout->addWidget(max_scattering_count_, 4, 1, 1, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    do_update_tracer_object();
}

ResourceWidget<tracer::Medium> *HeterogeneousWidget::clone()
{
    InitData init_data;
    init_data.transform = transform_->clone();
    init_data.density   = density_->clone();
    init_data.albedo    = albedo_->clone();
    init_data.g         = g_->clone();
    init_data.max_scattering_count = max_scattering_count_->value();
    return new HeterogeneousWidget(init_data, obj_ctx_);
}

Box<ResourceThumbnailProvider> HeterogeneousWidget::get_thumbnail(
    int width, int height) const
{
    return newBox<EmptyResourceThumbnailProvider>(width, height);
}

void HeterogeneousWidget::save_asset(AssetSaver &saver)
{
    transform_->save_asset(saver);

    density_->save_asset(saver);
    albedo_ ->save_asset(saver);
    g_      ->save_asset(saver);

    saver.write(uint32_t(max_scattering_count_->value()));
}

void HeterogeneousWidget::load_asset(AssetLoader &loader)
{
    transform_->load_asset(loader);

    density_->load_asset(loader);
    albedo_ ->load_asset(loader);
    g_      ->load_asset(loader);

    max_scattering_count_->setValue(int(loader.read<uint32_t>()));

    do_update_tracer_object();
}

RC<tracer::ConfigNode> HeterogeneousWidget::to_config(
    JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "heterogeneous");

    grp->insert_child("transform", transform_->to_config());
    grp->insert_child("density", density_->to_config(ctx));
    grp->insert_child("albedo", albedo_->to_config(ctx));
    grp->insert_child("g", g_->to_config(ctx));

    grp->insert_int("max_scattering_count", max_scattering_count_->value());

    return grp;
}

void HeterogeneousWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void HeterogeneousWidget::do_update_tracer_object()
{
    auto transform = transform_->get_transform();
    auto density   = density_->get_tracer_object();
    auto albedo    = albedo_->get_tracer_object();
    auto g         = g_->get_tracer_object();
    const int max_scattering_count = max_scattering_count_->value();

    tracer_object_ = create_heterogeneous_medium(
        transform, density, albedo, g, max_scattering_count);
}

ResourceWidget<tracer::Medium> *HeterogeneousWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new HeterogeneousWidget({}, obj_ctx);
}

AGZ_EDITOR_END
