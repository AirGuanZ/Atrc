#include <QGridLayout>

#include <agz/editor/medium/homogeneous.h>
#include <agz/tracer/create/medium.h>

AGZ_EDITOR_BEGIN

HomogeneousWidget::HomogeneousWidget(const InitData &init_data)
{
    sigma_a_ = new SpectrumInput(this);
    sigma_s_ = new SpectrumInput(this);
    g_ = new RealSlider(this, real(-0.99), real(0.99), 0);
    max_scattering_count_ = new QSpinBox(this);

    sigma_a_->set_value(init_data.sigma_a);
    sigma_s_->set_value(init_data.sigma_s);
    g_->set_value(init_data.g);
    g_->set_orientation(Qt::Horizontal);

    max_scattering_count_->setRange(0, (std::numeric_limits<int>::max)());
    max_scattering_count_->setValue(init_data.max_scattering_count);

    connect(sigma_a_, &SpectrumInput::edit_value, [=](const Spectrum&)
    {
        set_dirty_flag();
    });

    connect(sigma_s_, &SpectrumInput::edit_value, [=](const Spectrum &)
    {
        set_dirty_flag();
    });

    connect(g_, &RealSlider::change_value, [=](real)
    {
        set_dirty_flag();
    });

    connect(max_scattering_count_, qOverload<int>(&QSpinBox::valueChanged),
        [=](int)
    {
        set_dirty_flag();
    });

    QGridLayout *layout = new QGridLayout(this);

    int row = 0;
    layout->addWidget(new QLabel("A"), row, 0);
    layout->addWidget(sigma_a_, row, 1);

    ++row;
    layout->addWidget(new QLabel("S"), row, 0);
    layout->addWidget(sigma_s_, row, 1);

    ++row;
    layout->addWidget(new QLabel("g"), row, 0);
    layout->addWidget(g_, row, 1);

    ++row;
    layout->addWidget(new QLabel("Max Scattering Count"), row, 0);
    layout->addWidget(max_scattering_count_, row, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    do_update_tracer_object();
}

ResourceWidget<tracer::Medium> *HomogeneousWidget::clone()
{
    InitData init_data;
    init_data.sigma_a              = sigma_a_->get_value();
    init_data.sigma_s              = sigma_s_->get_value();
    init_data.g                    = g_->value();
    init_data.max_scattering_count = max_scattering_count_->value();
    return new HomogeneousWidget(init_data);
}

void HomogeneousWidget::save_asset(AssetSaver &saver)
{
    saver.write(sigma_a_->get_value());
    saver.write(sigma_s_->get_value());
    saver.write(g_->value());
    saver.write(uint32_t(max_scattering_count_->value()));
}

void HomogeneousWidget::load_asset(AssetLoader &loader)
{
    sigma_a_->set_value(loader.read<Spectrum>());
    sigma_s_->set_value(loader.read<Spectrum>());
    g_      ->set_value(loader.read<real>());

    max_scattering_count_->setValue(int(loader.read<uint32_t>()));

    do_update_tracer_object();
}

RC<tracer::ConfigNode> HomogeneousWidget::to_config(
    JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "homogeneous");

    grp->insert_child(
        "sigma_a", tracer::ConfigArray::from_spectrum(sigma_a_->get_value()));
    grp->insert_child(
        "sigma_s", tracer::ConfigArray::from_spectrum(sigma_s_->get_value()));
    grp->insert_real("g", g_->value());

    grp->insert_int("max_scattering_count", max_scattering_count_->value());

    return grp;
}

void HomogeneousWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void HomogeneousWidget::do_update_tracer_object()
{
    const Spectrum sigma_a = sigma_a_->get_value();
    const Spectrum sigma_s = sigma_s_->get_value();
    const real g           = real(g_->value());
    const int max_scattering_count = max_scattering_count_->value();

    tracer_object_ = tracer::create_homogeneous_medium(
        sigma_a, sigma_s, g, max_scattering_count);
}

ResourceWidget<tracer::Medium> *HomogeneousWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new HomogeneousWidget({});
}

AGZ_EDITOR_END
