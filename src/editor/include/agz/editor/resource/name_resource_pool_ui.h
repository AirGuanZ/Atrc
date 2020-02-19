#pragma once

#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class NameResourcePoolWidget : public QWidget
{
public:

    QPushButton *create    = nullptr;
    QPushButton *remove    = nullptr;
    QPushButton *duplicate = nullptr;
    QPushButton *edit      = nullptr;
    QPushButton *rename    = nullptr;

    QListWidget *name_list = nullptr;

    NameResourcePoolWidget()
    {
        QVBoxLayout *vlayout = new QVBoxLayout(this);

        QWidget *hlayout_widget = new QWidget(this);
        QHBoxLayout *hlayout    = new QHBoxLayout(hlayout_widget);

        create    = new QPushButton("Add",  hlayout_widget);
        remove    = new QPushButton("Del",  hlayout_widget);
        duplicate = new QPushButton("Copy", hlayout_widget);
        edit      = new QPushButton("Edit", hlayout_widget);
        rename    = new QPushButton("Name", hlayout_widget);

        QScrollArea *scroll_area = new QScrollArea(this);
        name_list = new QListWidget(scroll_area);

        vlayout->setAlignment(Qt::AlignTop);
        vlayout->addWidget(hlayout_widget);
        vlayout->addWidget(scroll_area);

        hlayout_widget->setContentsMargins(0, 0, 0, 0);
        hlayout->setContentsMargins(0, 0, 0, 0);

        hlayout->addWidget(create);
        hlayout->addWidget(remove);
        hlayout->addWidget(duplicate);
        hlayout->addWidget(edit);
        hlayout->addWidget(rename);

        scroll_area->setWidget(name_list);
        scroll_area->setWidgetResizable(true);
        name_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
};

AGZ_EDITOR_END
