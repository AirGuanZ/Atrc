#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/renderer/export/export_renderer_particle.h>

AGZ_EDITOR_BEGIN

ExportRendererParticle::ExportRendererParticle(QWidget *parent)
    : ExportRendererWidget(parent)
{
    min_depth_ = new QSpinBox(this);
    max_depth_ = new QSpinBox(this);
    cont_prob_ = new RealSlider(this);

    particles_per_task_  = new QSpinBox(this);
    particle_task_count_ = new QSpinBox(this);

    forward_spp_       = new QSpinBox(this);
    forward_task_size_ = new QSpinBox(this);

    worker_count_ = new QSpinBox(this);

    min_depth_->setRange(1, 20);
    min_depth_->setValue(5);

    max_depth_->setRange(1, 20);
    max_depth_->setValue(10);

    connect(min_depth_, qOverload<int>(&QSpinBox::valueChanged),
        [=](int new_min_depth)
    {
        if(new_min_depth > max_depth_->value())
        {
            max_depth_->blockSignals(true);
            max_depth_->setValue(new_min_depth);
            max_depth_->blockSignals(false);
        }
    });

    connect(max_depth_, qOverload<int>(&QSpinBox::valueChanged),
        [=](int new_max_depth)
    {
        if(new_max_depth < min_depth_->value())
        {
            min_depth_->blockSignals(true);
            min_depth_->setValue(new_max_depth);
            min_depth_->blockSignals(false);
        }
    });

    cont_prob_->set_orientation(Qt::Horizontal);
    cont_prob_->set_range(0, 1);
    cont_prob_->set_value(real(0.9));

    particles_per_task_->setRange(1, (std::numeric_limits<int>::max)());
    particles_per_task_->setValue(1000);

    particle_task_count_->setRange(1, (std::numeric_limits<int>::max)());
    particle_task_count_->setValue(100);

    forward_spp_->setRange(1, (std::numeric_limits<int>::max)());
    forward_spp_->setValue(10);

    forward_task_size_->setRange(1, (std::numeric_limits<int>::max)());
    forward_task_size_->setValue(32);
    
    worker_count_->setRange((std::numeric_limits<int>::lowest)(),
                            (std::numeric_limits<int>::max)());
    worker_count_->setValue(-1);

    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    auto add_row = [&](const QString &name, QWidget *w)
    {
        layout->addWidget(new QLabel(name), row, 0);
        layout->addWidget(w, row++, 1);
    };

    add_row("Min Depth", min_depth_);
    add_row("Max Depth", max_depth_);
    add_row("Cont Prob", cont_prob_);
    add_row("Particles per Task", particles_per_task_);
    add_row("Particle Task Count", particle_task_count_);
    add_row("Forward Samples per Pixel", forward_spp_);
    add_row("Forward Task Size", forward_task_size_);
    add_row("Thread Count", worker_count_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

RC<tracer::ConfigGroup> ExportRendererParticle::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();

    grp->insert_str("type", "particle");
    grp->insert_int("min_depth", min_depth_->value());
    grp->insert_int("max_depth", max_depth_->value());
    grp->insert_real("cont_prob", cont_prob_->value());
    grp->insert_int("particle_task_count", particle_task_count_->value());
    grp->insert_int("particles_per_task", particles_per_task_->value());
    grp->insert_int("forward_spp", forward_spp_->value());
    grp->insert_int("forward_task_grid_size", forward_task_size_->value());
    grp->insert_int("worker_count", worker_count_->value());

    return grp;
}

void ExportRendererParticle::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(min_depth_->value()));
    saver.write(int32_t(max_depth_->value()));
    saver.write(cont_prob_->value());
    saver.write(int32_t(particle_task_count_->value()));
    saver.write(int32_t(particles_per_task_->value()));
    saver.write(int32_t(forward_spp_->value()));
    saver.write(int32_t(forward_task_size_->value()));
    saver.write(int32_t(worker_count_->value()));
}

void ExportRendererParticle::load_asset(AssetLoader &loader)
{
    min_depth_->setValue(int(loader.read<int32_t>()));
    max_depth_->setValue(int(loader.read<int32_t>()));
    cont_prob_->set_value(loader.read<real>());
    particle_task_count_->setValue(int(loader.read<int32_t>()));
    particles_per_task_->setValue(int(loader.read<int32_t>()));
    forward_spp_->setValue(int(loader.read<int32_t>()));
    forward_task_size_->setValue(int(loader.read<int32_t>()));
    worker_count_->setValue(int(loader.read<int32_t>()));
}

AGZ_EDITOR_END
