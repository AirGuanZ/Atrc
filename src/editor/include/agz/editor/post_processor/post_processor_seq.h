#pragma once

#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>

#include <agz/editor/post_processor/post_processor.h>

AGZ_EDITOR_BEGIN

class PostProcessorSeq : public QDialog
{
public:

    explicit PostProcessorSeq(QWidget *parent = nullptr);

    RC<tracer::ConfigArray> to_config() const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

private slots:

    void add();

    void del();

private:

    void init_pp_creators();

    PostProcessorWidget *create_pp(const QString &name) const;

    std::map<QString, Box<PostProcessorWidgetCreator>> pp_creators_;

    QVBoxLayout *display_pp_layout_;
    
    std::map<QListWidgetItem *, PostProcessorWidget *> item2pp_;

    QListWidget *pp_list_;
};

AGZ_EDITOR_END
