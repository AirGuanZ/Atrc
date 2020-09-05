#include <QFormLayout>

#include <agz/editor/material/material_thumbnail.h>
#include <agz/editor/material/paper.h>
#include <agz/editor/ui/utility/collapsible.h>
#include <agz/tracer/create/material.h>

AGZ_EDITOR_BEGIN

PaperWidget::PaperWidget(const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    QFormLayout *layout = new QFormLayout(this);

    auto add_section = [=](
        const QString &title, QWidget *widget, bool open = false)
    {
        Collapsible *section = new Collapsible(this, title);
        section->set_content_widget(widget);
        layout->addRow(section);
        if(open)
            section->open();
    };

    auto add_double_spinbox = [=](
        QDoubleSpinBox **output, const QString &title,
        real minValue, real maxValue, real initValue)
    {
        *output = new QDoubleSpinBox(this);
        (*output)->setDecimals(5);
        (*output)->setRange(minValue, maxValue);
        (*output)->setValue(initValue);
        layout->addRow(title, *output);
        connect(*output, qOverload<double>(&QDoubleSpinBox::valueChanged),
                [=] { set_dirty_flag(); });
    };

    color_ = init_data.color;
    if(!color_)
        color_ = new Texture2DSlot(obj_ctx_, "Constant");
    add_section("Color", color_);
    color_->set_dirty_callback([=] { set_dirty_flag(); });

    add_double_spinbox(&gf_, "Front g", -1, 1, init_data.gf);
    add_double_spinbox(&gb_, "Back g", -1, 1, init_data.gb);
    add_double_spinbox(&wf_, "Front g Weight", 0, 1, init_data.wf);
    add_double_spinbox(&front_eta_, "Front IOR", 1.01f, 100, init_data.front_eta);
    add_double_spinbox(&back_eta_, "Back IOR", 1.01f, 100, init_data.back_eta);
    add_double_spinbox(&thickness_, "Thickness", 0.01f, 100, init_data.thickness);
    add_double_spinbox(&sigma_s_, "Sigma S", 0, 10000, init_data.sigma_s);
    add_double_spinbox(&sigma_a_, "Sigma A", 0, 10000, init_data.sigma_a);
    add_double_spinbox(&front_roughness_, "Front Roughness", 0, 1, init_data.front_roughness);
    add_double_spinbox(&back_roughness_, "Back Roughness", 0, 1, init_data.back_roughness);

    normal_map_ = init_data.normal_map;
    if(!normal_map_)
        normal_map_ = new NormalMapWidget({}, obj_ctx_);
    add_section("Normal Map", normal_map_);

    connect(normal_map_, &NormalMapWidget::change_params, [=]
    {
        set_dirty_flag();
    });

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    do_update_tracer_object();
}

ResourceWidget<tracer::Material> *PaperWidget::clone()
{
    InitData init_data;
    init_data.color           = color_->clone();
    init_data.gf              = real(gf_->value());
    init_data.gb              = real(gb_->value());
    init_data.wf              = real(wf_->value());
    init_data.front_eta       = real(front_eta_->value());
    init_data.back_eta        = real(back_eta_->value());
    init_data.thickness       = real(thickness_->value());
    init_data.sigma_s         = real(sigma_s_->value());
    init_data.sigma_a         = real(sigma_a_->value());
    init_data.front_roughness = real(front_roughness_->value());
    init_data.back_roughness  = real(back_roughness_->value());
    init_data.normal_map = normal_map_->clone();

    return new PaperWidget(init_data, obj_ctx_);
}

Box<ResourceThumbnailProvider> PaperWidget::get_thumbnail(
    int width, int height) const
{
    return newBox<MaterialThumbnailProvider>(width, height, tracer_object_);
}

void PaperWidget::save_asset(AssetSaver &saver)
{
    color_->save_asset(saver);
    saver.write(gf_->value());
    saver.write(gb_->value());
    saver.write(wf_->value());
    saver.write(front_eta_->value());
    saver.write(back_eta_->value());
    saver.write(thickness_->value());
    saver.write(sigma_s_->value());
    saver.write(sigma_a_->value());
    saver.write(front_roughness_->value());
    saver.write(back_roughness_->value());
    normal_map_->save_asset(saver);
}

void PaperWidget::load_asset(AssetLoader &loader)
{
    auto load_double = [&](QDoubleSpinBox *result)
    {
        result->blockSignals(true);
        result->setValue(loader.read<double>());
        result->blockSignals(false);
    };

    color_->load_asset(loader);
    load_double(gf_);
    load_double(gb_);
    load_double(wf_);
    load_double(front_eta_);
    load_double(back_eta_);
    load_double(thickness_);
    load_double(sigma_s_);
    load_double(sigma_a_);
    load_double(front_roughness_);
    load_double(back_roughness_);
    normal_map_->load_asset(loader);

    do_update_tracer_object();
}

RC<tracer::ConfigNode> PaperWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "paper");

    grp->insert_child("color", color_->to_config(ctx));
    grp->insert_real("gf", real(gf_->value()));
    grp->insert_real("gb", real(gb_->value()));
    grp->insert_real("wf", real(wf_->value()));
    grp->insert_real("wb", 1 - real(wf_->value()));
    grp->insert_real("front_eta", real(front_eta_->value()));
    grp->insert_real("back_eta", real(back_eta_->value()));
    grp->insert_real("d", real(thickness_->value()));
    grp->insert_real("sigma_s", real(sigma_s_->value()));
    grp->insert_real("sigma_a", real(sigma_a_->value()));
    grp->insert_real("front_roughness", real(front_roughness_->value()));
    grp->insert_real("back_roughness", real(back_roughness_->value()));

    if(normal_map_->is_enabled())
        grp->insert_child("normal", normal_map_->to_config(ctx));

    return grp;
}

void PaperWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void PaperWidget::do_update_tracer_object()
{
    auto color = color_->get_tracer_object();
    const real gf = real(gf_->value());
    const real gb = real(gb_->value());
    const real wf = real(wf_->value());
    const real wb = 1 - wf;
    const real front_eta = real(front_eta_->value());
    const real back_eta = real(back_eta_->value());
    const real thickness = real(thickness_->value());
    const real sigma_s = real(sigma_s_->value());
    const real sigma_a = real(sigma_a_->value());
    const real front_roughness = real(front_roughness_->value());
    const real back_roughness = real(back_roughness_->value());
    auto normal_map = normal_map_->get_tracer_object();

    tracer_object_ = create_paper(
        std::move(color),
        gf, gb, wf, wb,
        front_eta, back_eta,
        thickness, sigma_s, sigma_a,
        front_roughness, back_roughness,
        std::move(normal_map));
}

ResourceWidget<tracer::Material> *PaperWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new PaperWidget({}, obj_ctx);
}

AGZ_EDITOR_END
