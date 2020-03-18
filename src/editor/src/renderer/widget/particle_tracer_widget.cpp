#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/renderer/widget/particle_tracer_widget.h>
#include <agz/editor/renderer/particle_tracer.h>

AGZ_EDITOR_BEGIN

ParticleTracerWidget::ParticleTracerWidget(QWidget *parent)
    : RendererWidget(parent)
{
    min_depth_ = new QSlider(Qt::Horizontal, this);
    max_depth_ = new QSlider(Qt::Horizontal, this);
    cont_prob_ = new QSlider(Qt::Horizontal, this);
    particle_sample_count_ = new QSlider(Qt::Horizontal, this);

    QLabel *min_depth_text = new QLabel("Min Depth", this);
    QLabel *max_depth_text = new QLabel("Max Depth", this);
    QLabel *cont_prob_text = new QLabel("Cont Prob", this);
    QLabel *particle_sc_text = new QLabel("Sample Count", this);

    QLabel *min_depth_display = new QLabel(this);
    QLabel *max_depth_display = new QLabel(this);
    QLabel *cont_prob_display = new QLabel(this);
    QLabel *particle_sc_display = new QLabel(this);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(min_depth_text, 0, 0);
    layout->addWidget(min_depth_display, 0, 1);
    layout->addWidget(min_depth_, 0, 2);

    layout->addWidget(max_depth_text, 1, 0);
    layout->addWidget(max_depth_display, 1, 1);
    layout->addWidget(max_depth_, 1, 2);

    layout->addWidget(cont_prob_text, 2, 0);
    layout->addWidget(cont_prob_display, 2, 1);
    layout->addWidget(cont_prob_, 2, 2);

    layout->addWidget(particle_sc_text, 3, 0);
    layout->addWidget(particle_sc_display, 3, 1);
    layout->addWidget(particle_sample_count_, 3, 2);

    min_depth_display->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    max_depth_display->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    cont_prob_display->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    particle_sc_display->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    min_depth_display->setAlignment(Qt::AlignCenter);
    max_depth_display->setAlignment(Qt::AlignCenter);
    cont_prob_display->setAlignment(Qt::AlignCenter);
    particle_sc_display->setAlignment(Qt::AlignCenter);

    min_depth_->setRange(1, 20);
    max_depth_->setRange(1, 20);
    cont_prob_->setRange(1, 10);
    particle_sample_count_->setRange(1, 20);

    min_depth_->setValue(5);
    max_depth_->setValue(10);
    cont_prob_->setValue(9);
    particle_sample_count_->setValue(4);

    min_depth_display->setText("5");
    max_depth_display->setText("10");
    cont_prob_display->setText("0.9");
    particle_sc_display->setText("4");

    connect(min_depth_, &QSlider::valueChanged, [=](int new_value)
    {
        min_depth_display->setText(QString::number(new_value));
        emit change_renderer_params();
    });

    connect(max_depth_, &QSlider::valueChanged, [=](int new_value)
    {
        max_depth_display->setText(QString::number(new_value));
        emit change_renderer_params();
    });

    connect(cont_prob_, &QSlider::valueChanged, [=](int new_value)
    {
        const real new_cont_prob = new_value / real(10);
        cont_prob_display->setText(QString::number(new_cont_prob));
        emit change_renderer_params();
    });

    connect(particle_sample_count_, &QSlider::valueChanged, [=](int new_value)
    {
        particle_sc_display->setText(QString::number(new_value));
        emit change_renderer_params();
    });
}

Box<Renderer> ParticleTracerWidget::create_renderer(
    RC<tracer::Scene> scene, const Vec2i &framebuffer_size,
    bool enable_preview) const
{
    ParticleTracer::Params params = {
        -2, 32,
        particle_sample_count_->value(),
        min_depth_->value(),
        max_depth_->value(),
        cont_prob_->value() / real(10),
        enable_preview
    };
    return newBox<ParticleTracer>(
        params, framebuffer_size.x, framebuffer_size.y, std::move(scene));
}

void ParticleTracerWidget::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(particle_sample_count_->value()));
    saver.write(int32_t(min_depth_->value()));
    saver.write(int32_t(max_depth_->value()));
    saver.write(int32_t(cont_prob_->value()));
}

void ParticleTracerWidget::load_asset(AssetLoader &loader)
{
    particle_sample_count_->setValue(int(loader.read<int32_t>()));
    min_depth_->setValue(int(loader.read<int32_t>()));
    max_depth_->setValue(int(loader.read<int32_t>()));
    cont_prob_->setValue(int(loader.read<int32_t>()));

    emit change_renderer_params();
}

RendererWidget *ParticleTracerWidgetCreator::create_widget(QWidget *parent) const
{
    return new ParticleTracerWidget(parent);
}

AGZ_EDITOR_END
