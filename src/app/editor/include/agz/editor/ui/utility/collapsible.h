#pragma once

#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class Collapsible : public QWidget
{
    Q_OBJECT

public:

    Collapsible(QWidget *parent, const QString &title, int ani_duration_ms = 100);

    void set_content_layout(QLayout *content_layout);

private slots:

    void toggle(bool collapsed);

private:

    QGridLayout *layout_;

    QToolButton *toggle_button_;
    QParallelAnimationGroup *animation_;
    QScrollArea *content_;

    int ani_duration_ms_;
};

AGZ_EDITOR_END
