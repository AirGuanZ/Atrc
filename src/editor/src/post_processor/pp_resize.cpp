#include <QGridLayout>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/post_processor/pp_resize.h>

AGZ_EDITOR_BEGIN

ResizeWidget::ResizeWidget(const PostProcessorWidgetCreator *creator)
    : PostProcessorWidget(creator)
{
    width_ = new QSpinBox(this);
    height_ = new QSpinBox(this);

    width_->setRange(1, (std::numeric_limits<int>::max)());
    height_->setRange(1, (std::numeric_limits<int>::max)());

    width_->setValue(640);
    height_->setValue(480);

    auto layout = new QGridLayout(this);

    layout->addWidget(new QLabel("Width"), 0, 0);
    layout->addWidget(width_, 0, 1);

    layout->addWidget(new QLabel("Height"), 1, 0);
    layout->addWidget(height_, 1, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
}

RC<tracer::ConfigGroup> ResizeWidget::to_config() const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "resize");
    grp->insert_child("size", tracer::ConfigArray::from_vec2i({
        width_->value(), height_->value()
    }));
    return grp;
}

void ResizeWidget::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(width_->value()));
    saver.write(int32_t(height_->value()));
}

void ResizeWidget::load_asset(AssetLoader &loader)
{
    width_->setValue(int(loader.read<int32_t>()));
    height_->setValue(int(loader.read<int32_t>()));
}

PostProcessorWidget *ResizeWidgetCreator::create() const
{
    return new ResizeWidget(this);
}

AGZ_EDITOR_END
