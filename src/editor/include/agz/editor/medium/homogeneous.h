#pragma once

#include <QSpinBox>

#include <agz/editor/medium/medium.h>
#include <agz/editor/ui/utility/adaptive_slider.h>
#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

class HomogeneousWidget : public MediumWidget
{
public:

    struct InitData
    {
        Spectrum sigma_a;
        Spectrum sigma_s;
        real g = 0;

        int max_scattering_count = (std::numeric_limits<int>::max)();
    };

    explicit HomogeneousWidget(const InitData &init_data);

    ResourceWidget<tracer::Medium> *clone() override;

    void save_asset(AssetSaver &saver) override;

    void load_asset(AssetLoader &loader) override;

protected:

    void update_tracer_object_impl() override;

private:

    void do_update_tracer_object();

    SpectrumInput *sigma_a_ = nullptr;
    SpectrumInput *sigma_s_ = nullptr;
    RealSlider *g_ = nullptr;

    QSpinBox *max_scattering_count_ = nullptr;
};

class HomogeneousWidgetCreator : public MediumWidgetCreator
{
public:

    QString name() const override
    {
        return "Homogeneous";
    }

    ResourceWidget<tracer::Medium> *create_widget(ObjectContext &obj_ctx) const override;
};

AGZ_EDITOR_END
