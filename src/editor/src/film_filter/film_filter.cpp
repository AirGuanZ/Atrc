#include <agz/editor/film_filter/film_filter.h>
#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>

#include <agz/editor/film_filter/box.h>
#include <agz/editor/film_filter/gaussian.h>

AGZ_EDITOR_BEGIN

FilmFilterWidget::FilmFilterWidget(const FilmFilterWidgetCreator *creator)
    : creator_(creator)
{

}

QString FilmFilterWidget::get_type() const
{
    return creator_->get_type();
}

FilmFilterPanel::FilmFilterPanel(QWidget *parent)
    : QDialog(parent)
{
    init_creators();

    layout_ = new QVBoxLayout(this);
    type_selector_ = new QComboBox(this);
    layout_->addWidget(type_selector_);

    QStringList types;
    for(auto &p : creators_)
        types.push_back(p.first);
    type_selector_->addItems(types);

    type_selector_->setCurrentText("Box");
    change_widget_type("Box");

    connect(
        type_selector_, &QComboBox::currentTextChanged,
        [=](const QString &new_type)
    {
        change_widget_type(new_type);
    });
}

RC<tracer::ConfigGroup> FilmFilterPanel::to_config() const
{
    return widget_->to_config();
}

void FilmFilterPanel::save_asset(AssetSaver &saver) const
{
    saver.write_string(widget_->get_type());
    widget_->save_asset(saver);
}

void FilmFilterPanel::load_asset(AssetLoader &loader)
{
    const QString type = loader.read_string();

    type_selector_->blockSignals(true);
    type_selector_->setCurrentText(type);
    type_selector_->blockSignals(false);

    change_widget_type(type);
    widget_->load_asset(loader);
}

void FilmFilterPanel::init_creators()
{
    auto add_creator = [&](Box<FilmFilterWidgetCreator> creator)
    {
        const QString type = creator->get_type();
        creators_[type] = std::move(creator);
    };

    add_creator(newBox<BoxWidgetCreator>());
    add_creator(newBox<GaussianWidgetCreator>());
}

void FilmFilterPanel::change_widget_type(const QString &new_type)
{
    if(widget_)
        delete widget_;
    widget_ = creators_[new_type]->create();
    layout_->addWidget(widget_);
}

AGZ_EDITOR_END
