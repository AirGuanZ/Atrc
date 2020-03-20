#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/post_processor/post_processor_seq.h>

#include <agz/editor/post_processor/pp_aces.h>
#include <agz/editor/post_processor/pp_gamma.h>
#include <agz/editor/post_processor/pp_oidn_denoise.h>
#include <agz/editor/post_processor/pp_resize.h>
#include <agz/editor/post_processor/pp_save_to_img.h>
#include <agz/editor/post_processor/pp_save_gbuffer.h>

AGZ_EDITOR_BEGIN

namespace
{
    constexpr int WIDGET_ITEM_HEIGHT = 35;
}

PostProcessorSeq::PostProcessorSeq(QWidget *parent)
    : QDialog(parent)
{
    auto right_widget = new QWidget(this);
    display_pp_layout_ = new QVBoxLayout(right_widget);

    auto left_widget = new QWidget(this);
    auto left_layout = new QGridLayout(left_widget);
    auto new_button = new QPushButton("Add", left_widget);
    auto del_button = new QPushButton("Del", left_widget);

    pp_list_ = new QListWidget(left_widget);
    pp_list_->setDragDropMode(QAbstractItemView::InternalMove);

    left_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    left_layout->addWidget(new_button, 0, 0, 1, 1);
    left_layout->addWidget(del_button, 0, 1, 1, 1);
    left_layout->addWidget(pp_list_, 1, 0, 1, 2);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(left_widget);
    layout->addWidget(right_widget);

    connect(new_button, &QPushButton::clicked,
            this,       &PostProcessorSeq::add);

    connect(del_button, &QPushButton::clicked,
            this,       &PostProcessorSeq::del);

    connect(pp_list_, &QListWidget::currentItemChanged,
            [=](QListWidgetItem *cur, QListWidgetItem *old)
    {
        if(auto it = item2pp_.find(old); it != item2pp_.end())
            it->second->hide();

        if(auto it = item2pp_.find(cur); it != item2pp_.end())
            it->second->show();
    });

    init_pp_creators();

    {
        auto init_pp = create_pp("Save to Image");
        display_pp_layout_->addWidget(init_pp);
        init_pp->hide();

        pp_list_->insertItem(pp_list_->count(), init_pp->get_type());
        auto item = pp_list_->item(pp_list_->count() - 1);
        item2pp_[item] = init_pp;

        item->setSizeHint(
            QSize(item->sizeHint().width(), WIDGET_ITEM_HEIGHT));
    }
}

RC<tracer::ConfigArray> PostProcessorSeq::to_config() const
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

void PostProcessorSeq::save_asset(AssetSaver &saver) const
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

void PostProcessorSeq::load_asset(AssetLoader &loader)
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

        display_pp_layout_->addWidget(pp);
        pp->hide();

        pp_list_->insertItem(pp_list_->count(), type);
        auto item = pp_list_->item(pp_list_->count() - 1);
        item2pp_[item] = pp;

        item->setSizeHint(
            QSize(item->sizeHint().width(), WIDGET_ITEM_HEIGHT));
    }
}

void PostProcessorSeq::add()
{
    QStringList type_list;
    for(auto &c : pp_creators_)
        type_list.push_back(c.second->get_type());

    bool ok = false;
    const QString type = QInputDialog::getItem(
        this, "Select Type", "Select type of new post processor",
        type_list, 0, false, &ok);
    if(!ok)
        return;

    auto pp = create_pp(type);
    display_pp_layout_->addWidget(pp);
    pp->hide();

    pp_list_->insertItem(pp_list_->count(), type);
    auto item = pp_list_->item(pp_list_->count() - 1);
    item2pp_[item] = pp;

    item->setSizeHint(
        QSize(item->sizeHint().width(), WIDGET_ITEM_HEIGHT));
}

void PostProcessorSeq::del()
{
    auto item = pp_list_->currentItem();
    if(!item)
        return;

    auto it = item2pp_.find(item);
    if(it != item2pp_.end())
    {
        delete it->second;
        item2pp_.erase(it);
    }

    delete pp_list_->takeItem(pp_list_->row(item));
}

void PostProcessorSeq::init_pp_creators()
{
    auto add_creator = [&](Box<PostProcessorWidgetCreator> creator)
    {
        auto type = creator->get_type();
        pp_creators_[type] = std::move(creator);
    };

    add_creator(newBox<ACESWidgetCreator>());
    add_creator(newBox<GammaWidgetCreator>());
    add_creator(newBox<ResizeWidgetCreator>());
    add_creator(newBox<SaveGBufferWidgetCreator>());
    add_creator(newBox<SaveToImageWidgetCreator>());

#ifdef USE_OIDN
    add_creator(newBox<OIDNWidgetCreator>());
#endif
}

PostProcessorWidget *PostProcessorSeq::create_pp(const QString &name) const
{
    auto it = pp_creators_.find(name);
    assert(it != pp_creators_.end());
    return it->second->create();
}

AGZ_EDITOR_END
