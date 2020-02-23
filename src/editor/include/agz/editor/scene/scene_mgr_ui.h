#pragma once

#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class SceneManagerWidget : public QWidget
{
public:

    QPushButton *create = nullptr;
    QPushButton *remove = nullptr;
    QPushButton *rename = nullptr;
    QPushButton *import = nullptr;

    QListWidget *name_list = nullptr;

    SceneManagerWidget()
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        QWidget     *buttons_widget = new QWidget(this);
        QHBoxLayout *buttons_layout = new QHBoxLayout(buttons_widget);

        create = new QPushButton("Add",  buttons_widget);
        remove = new QPushButton("Del",  buttons_widget);
        rename = new QPushButton("Name", buttons_widget);
        import = new QPushButton("Import", buttons_widget);

        name_list = new QListWidget(this);
        name_list->setDragDropMode(QAbstractItemView::InternalMove);

        buttons_widget->setContentsMargins(0, 0, 0, 0);
        buttons_layout->setContentsMargins(0, 0, 0, 0);
        buttons_layout->addWidget(create);
        buttons_layout->addWidget(remove);
        buttons_layout->addWidget(rename);
        buttons_layout->addWidget(import);

        layout->addWidget(buttons_widget);
        layout->addWidget(name_list);
    }
};

AGZ_EDITOR_END
