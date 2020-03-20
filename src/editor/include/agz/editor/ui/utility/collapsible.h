#pragma once

#include <QGridLayout>
#include <QToolButton>
#include <QTreeWidget>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class Collapsible : public QWidget
{
    Q_OBJECT

public:

    Collapsible(QWidget *parent, const QString &title);

    void set_content_widget(QWidget *widget);

    void open();

    void set_toggle_button_disabled(bool disabled);

private slots:

    void toggle(bool collapsed);

private:

    QVBoxLayout *layout_;

    QToolButton *toggle_button_;
    QWidget     *content_ = nullptr;
};

AGZ_EDITOR_END
