#include <QFileDialog>
#include <QGridLayout>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/ui/save_filename.h>

AGZ_EDITOR_BEGIN

SaveFilenameWidget::SaveFilenameWidget(QWidget *parent)
    : QWidget(parent)
{
    relative_to_scene_ = new QCheckBox("Relative", this);
    browse_            = new QPushButton("Browse", this);
    browsed_filename_  = new ElidedLabel("", this);
    relative_filename_ = new QLineEdit(this);

    relative_to_scene_->setChecked(true);

    browse_          ->setDisabled(true);
    browsed_filename_->setDisabled(true);

    auto layout = new QGridLayout(this);
    layout->addWidget(browse_, 0, 0);
    layout->addWidget(browsed_filename_, 0, 1);
    layout->addWidget(relative_to_scene_, 1, 0);
    layout->addWidget(relative_filename_, 1, 1);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(relative_to_scene_, &QCheckBox::stateChanged, [=](int)
    {
        if(relative_to_scene_->isChecked())
        {
            browse_           ->setDisabled(true);
            browsed_filename_ ->setDisabled(true);
            relative_filename_->setDisabled(false);
        }
        else
        {
            browse_           ->setDisabled(false);
            browsed_filename_ ->setDisabled(false);
            relative_filename_->setDisabled(true);
        }
    });

    connect(browse_, &QPushButton::clicked,
            this, &SaveFilenameWidget::browse_filename);
}

void SaveFilenameWidget::set_relative_filename(const QString &filename)
{
    relative_filename_->setText(filename);
}

void SaveFilenameWidget::set_browse_filter(const QString &filter)
{
    browse_filter_ = filter;
}

QString SaveFilenameWidget::get_filename(const QString &relative_prefix) const
{
    if(relative_to_scene_->isChecked())
        return relative_prefix + relative_filename_->text();
    return browsed_filename_->text();
}

void SaveFilenameWidget::save_asset(AssetSaver &saver) const
{
    saver.write_string(browsed_filename_->text());
    saver.write(int32_t(relative_to_scene_->isChecked() ? 1 : 0));
    saver.write_string(relative_filename_->text());
}

void SaveFilenameWidget::load_asset(AssetLoader &loader)
{
    browsed_filename_->setText(loader.read_string());
    browsed_filename_->setToolTip(browsed_filename_->text());
    relative_to_scene_->setChecked(loader.read<int32_t>() != 0);
    relative_filename_->setText(loader.read_string());
}

void SaveFilenameWidget::browse_filename()
{
    const QString new_filename = QFileDialog::getSaveFileName(
        this, QString(), QString(), browse_filter_);
    if(new_filename.isEmpty())
        return;

    browsed_filename_->setText(new_filename);
    browsed_filename_->setToolTip(new_filename);
}

AGZ_EDITOR_END
