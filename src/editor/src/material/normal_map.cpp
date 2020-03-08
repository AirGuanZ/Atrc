#include <QCheckBox>

#include <agz/editor/material/normal_map.h>

AGZ_EDITOR_BEGIN

NormalMapWidget::NormalMapWidget(const InitData &init_data, ObjectContext &obj_ctx)
    : obj_ctx_(obj_ctx)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    apply_normal_map_ = new QCheckBox("Apply normal map", this);
    apply_normal_map_->setChecked(init_data.apply_normal_map);

    normal_map_ = init_data.normal_map;
    if(!normal_map_)
        normal_map_ = new Texture2DSlot(obj_ctx_, "Image");
    normal_map_->setDisabled(init_data.apply_normal_map ? false : true);

    layout->addWidget(apply_normal_map_);
    layout->addWidget(normal_map_);

    connect(apply_normal_map_, &QCheckBox::stateChanged, [=](int)
    {
        if(apply_normal_map_->isChecked())
            normal_map_->setDisabled(false);
        else
            normal_map_->setDisabled(true);

        emit change_params();
    });

    normal_map_->set_dirty_callback([=]
    {
        emit change_params();
    });
}

NormalMapWidget *NormalMapWidget::clone() const
{
    InitData init_data;
    init_data.apply_normal_map = apply_normal_map_->isChecked();
    init_data.normal_map       = normal_map_->clone();
    return new NormalMapWidget(init_data, obj_ctx_);
}

void NormalMapWidget::save_asset(AssetSaver &saver)
{
    saver.write(uint8_t(apply_normal_map_->isChecked() ? 1 : 0));
    normal_map_->save_asset(saver);
}

void NormalMapWidget::load_asset(AssetLoader &loader)
{
    const bool apply = loader.read<uint8_t>() != 0;
    apply_normal_map_->setChecked(apply);
    normal_map_->load_asset(loader);
}

std::unique_ptr<tracer::NormalMapper> NormalMapWidget::get_tracer_object() const
{
    if(apply_normal_map_->isChecked())
    {
        auto normal_map = normal_map_->get_tracer_object();
        return std::make_unique<tracer::NormalMapper>(std::move(normal_map));
    }
    return std::make_unique<tracer::NormalMapper>(nullptr);
}

bool NormalMapWidget::is_enabled() const noexcept
{
    return apply_normal_map_->isChecked();
}

std::shared_ptr<tracer::ConfigNode> NormalMapWidget::to_config(JSONExportContext &ctx) const
{
    return normal_map_->to_config(ctx);
}

AGZ_EDITOR_END
