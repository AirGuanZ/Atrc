#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

#include <agz/editor/ui/utility/elided_label.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class SaveFilenameWidget : public QWidget
{
public:

    explicit SaveFilenameWidget(QWidget *parent = nullptr);

    void set_relative_filename(const QString &filename);

    void set_browse_filter(const QString &filter);

    QString get_filename(const QString &relative_prefix) const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

private slots:

    void browse_filename();

private:

    QCheckBox *relative_to_scene_ = nullptr;

    QPushButton *browse_ = nullptr;
    ElidedLabel *browsed_filename_ = nullptr;

    QString browse_filter_;

    QLineEdit *relative_filename_ = nullptr;
};

AGZ_EDITOR_END
