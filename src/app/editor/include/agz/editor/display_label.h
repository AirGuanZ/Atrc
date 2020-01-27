#pragma once

#include <QLabel>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class DisplayLabel : public QLabel
{
    Q_OBJECT

public:

    explicit DisplayLabel(QWidget *parent)
        : QLabel(parent)
    {
        setAlignment(Qt::AlignCenter);
        setScaledContents(true);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

signals:

    void resize();

protected:

    void resizeEvent(QResizeEvent *event) override
    {
        emit resize();
    }
};

AGZ_EDITOR_END
