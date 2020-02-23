#pragma once

#include <QComboBox>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

// customed combo box which doesn't change when mouse wheel scrolls on it
class ComboBoxWithoutWheelFocus : public QComboBox
{
public:

    explicit ComboBoxWithoutWheelFocus(QWidget *parent = nullptr)
        : QComboBox(parent)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

protected:

    void wheelEvent(QWheelEvent *e)
    {
        if(hasFocus())
            QComboBox::wheelEvent(e);
    }
};

AGZ_EDITOR_END
