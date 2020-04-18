#pragma once

#include <QComboBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QWidget>

#include <agz/editor/common.h>
#include <agz/tracer/utility/config.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;

class FilmFilterWidgetCreator;

class FilmFilterWidget : public QWidget
{
    const FilmFilterWidgetCreator *creator_;

public:

    explicit FilmFilterWidget(const FilmFilterWidgetCreator *creator);

    virtual ~FilmFilterWidget() = default;

    QString get_type() const;

    virtual RC<tracer::ConfigGroup> to_config() const = 0;

    virtual void save_asset(AssetSaver &saver) const = 0;

    virtual void load_asset(AssetLoader &loader) = 0;
};

class FilmFilterWidgetCreator
{
public:

    virtual ~FilmFilterWidgetCreator() = default;

    virtual FilmFilterWidget *create() const = 0;

    virtual QString get_type() const = 0;
};

class FilmFilterPanel : public QDialog
{
public:

    explicit FilmFilterPanel(QWidget *parent);

    RC<tracer::ConfigGroup> to_config() const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

private:

    void init_creators();

    void change_widget_type(const QString &new_type);

    QComboBox *type_selector_ = nullptr;

    QVBoxLayout *layout_ = nullptr;

    FilmFilterWidget *widget_ = nullptr;

    std::map<QString, Box<FilmFilterWidgetCreator>> creators_;
};

AGZ_EDITOR_END
