#pragma once

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class Texture2DPoolWidget : public QWidget
{
public:

    QPushButton *create    = nullptr;
    QPushButton *remove    = nullptr;
    QPushButton *duplicate = nullptr;

    QWidget *scroll_area_widget = nullptr;

    Texture2DPoolWidget()
    {
        QVBoxLayout *vlayout = new QVBoxLayout(this);

        QWidget *hlayout_widget = new QWidget(this);
        QHBoxLayout *hlayout = new QHBoxLayout(hlayout_widget);

        create    = new QPushButton("Add",  hlayout_widget);
        remove    = new QPushButton("Del",  hlayout_widget);
        duplicate = new QPushButton("Copy", hlayout_widget);

        QScrollArea *scroll_area = new QScrollArea(this);
        scroll_area_widget       = new QWidget(scroll_area);

        vlayout->setAlignment(Qt::AlignTop);
        vlayout->addWidget(hlayout_widget);
        vlayout->addWidget(scroll_area);

        hlayout_widget->setContentsMargins(0, 0, 0, 0);
        hlayout->setContentsMargins(0, 0, 0, 0);

        hlayout->addWidget(create);
        hlayout->addWidget(remove);
        hlayout->addWidget(duplicate);

        scroll_area->setWidget(scroll_area_widget);
        scroll_area->setWidgetResizable(true);
        scroll_area_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        scroll_area_widget->setContentsMargins(0, 0, 0, 0);

        setContentsMargins(0, 0, 0, 0);
        vlayout->setContentsMargins(0, 0, 0, 0);
    }
};

AGZ_EDITOR_END
