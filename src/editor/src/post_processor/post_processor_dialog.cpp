#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/post_processor/post_processor_dialog.h>

AGZ_EDITOR_BEGIN

PostProcessorDialog::PostProcessorDialog(QWidget *parent)
    : QWidget(parent)
{
    auto right_area = new QScrollArea(this);
    auto right_widget = new QWidget(right_area);
    right_area->setWidget(right_widget);
    right_area->setWidgetResizable(true);
    display_pp_layout_ = new QVBoxLayout(right_widget);

    auto left_widget = new QWidget(this);
    auto left_layout = QGridLayout(left_widget);
    auto new_button = new QPushButton("Add", this);
    auto del_button = new QPushButton("Del", this);
    pp_list_ = new QListWidget(this);

    left_layout.addWidget(new_button, 0, 0, 1, 1);
    left_layout.addWidget(del_button, 0, 1, 1, 1);
    left_layout.addWidget(pp_list_, 1, 0, 1, 2);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(left_widget);
    layout->addWidget(right_area);

    connect(new_button, &QPushButton::clicked,
            this,       &PostProcessorDialog::add);

    connect(del_button, &QPushButton::clicked,
            this,       &PostProcessorDialog::del);

    connect(pp_list_, &QListWidget::currentItemChanged,
            [=](QListWidgetItem *cur, QListWidgetItem *old)
    {
        if(auto it = item2pp_.find(old); it != item2pp_.end())
            it->second->hide();

        if(auto it = item2pp_.find(cur); it != item2pp_.end())
            it->second->show();
    });
}

RC<tracer::ConfigArray> PostProcessorDialog::to_config() const
{
    auto arr = newRC<tracer::ConfigArray>();

    for(int row = 0; row < pp_list_->count(); ++row)
    {
        auto item = pp_list_->item(row);
        auto item_it = item2pp_.find(item);

        if(item_it != item2pp_.end())
            arr->push_back(item_it->second->to_config());
    }

    return arr;
}

void PostProcessorDialog::save_asset(AssetSaver &saver) const
{
    saver.write(int32_t(pp_list_->count()));

    for(int row = 0; row < pp_list_->count();  ++row)
    {
        auto item = pp_list_->item(row);
        auto item_it = item2pp_.find(item);

        if(item_it != item2pp_.end())
        {
            auto pp = item_it->second;
            saver.write_string(pp->get_type());
            pp->save_asset(saver);
        }
    }
}

void PostProcessorDialog::load_asset(AssetLoader &loader)
{
    // clear all existing pp

    pp_list_->clear();
    for(auto &p : item2pp_)
        delete p.second;
    item2pp_.clear();
    
    // load new pp

    const int count = int(loader.read<int32_t>());
    for(int i = 0; i < count; ++i)
    {
        const QString type = loader.read_string();
        auto pp = create_pp(type);
        pp->load_asset(loader);

        pp->hide();

        pp_list_->insertItem(pp_list_->count(), type);
        auto item = pp_list_->item(pp_list_->count() - 1);
        item2pp_[item] = pp;
    }
}

void PostProcessorDialog::add()
{
    // TODO
}

void PostProcessorDialog::del()
{
    // TODO
}

PostProcessorWidget *PostProcessorDialog::create_pp(const QString &name) const
{
    // TODO
    return nullptr;
}

AGZ_EDITOR_END
