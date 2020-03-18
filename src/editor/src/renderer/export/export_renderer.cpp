#include <QVBoxLayout>

#include <agz/editor/renderer/export/export_renderer.h>
#include <agz/editor/renderer/export/export_renderer_ao.h>
#include <agz/editor/renderer/export/export_renderer_bdpt.h>
#include <agz/editor/renderer/export/export_renderer_particle.h>
#include <agz/editor/renderer/export/export_renderer_pssmlt_pt.h>
#include <agz/editor/renderer/export/export_renderer_pt.h>
#include <agz/editor/renderer/export/export_renderer_sppm.h>

AGZ_EDITOR_BEGIN

namespace
{
    ExportRendererWidget *create_widget(const QString &type, QWidget *parent)
    {
        if(type == "AO")
            return new ExportRendererAO(parent);
        if(type == "BDPT")
            return new ExportRendererBDPT(parent);
        if(type == "Particle")
            return new ExportRendererParticle(parent);
        if(type == "PSSMLT PT")
            return new ExportRendererPSSMLTPT(parent);
        if(type == "PT")
            return new ExportRendererPT(parent);
        if(type == "SPPM")
            return new ExportRendererSPPM(parent);
        return new ExportRendererPT(parent);
    }
}

ExportRendererPanel::ExportRendererPanel()
{
    type_selector_ = new QComboBox(this);
    type_selector_->addItems({ "AO", "BDPT", "Particle", "PSSMLT PT", "PT", "SPPM" });
    type_selector_->setCurrentText("PT");

    renderer_widget_ = create_widget(type_selector_->currentText(), this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(type_selector_);
    layout->addWidget(renderer_widget_);

    connect(type_selector_, &QComboBox::currentTextChanged,
        [=](const QString &new_type)
    {
        assert(renderer_widget_);
        delete renderer_widget_;
        renderer_widget_ = create_widget(new_type, this);
        layout->addWidget(renderer_widget_);
    });
}

RC<tracer::ConfigGroup> ExportRendererPanel::to_config() const
{
    assert(renderer_widget_);
    return renderer_widget_->to_config();
}

AGZ_EDITOR_END
