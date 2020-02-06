#pragma once

#include <QColorDialog>
#include <QPushButton>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class ColorHolder : public QPushButton
{
    Q_OBJECT

public:

    explicit ColorHolder(const Spectrum &init_color, QWidget *parent = nullptr)
        : QPushButton(parent)
    {
        setFlat(true);
        setAutoFillBackground(true);

        color_ = QColor::fromRgbF(init_color.r, init_color.g, init_color.b);
        set_displayed_color(color_);

        connect(this, &QPushButton::clicked, [=]
        {
            QColor new_color = QColorDialog::getColor(color_, this);
            if(new_color.isValid())
            {
                color_ = new_color;
                set_displayed_color(color_);
                emit change_color(get_color());
            }
        });
    }

    Spectrum get_color() const noexcept
    {
        return qcolor_to_spectrum(color_);
    }

signals:

    void change_color(const Spectrum &new_color);

private:

    QColor color_;

    void set_displayed_color(const QColor &color)
    {
        QPalette pal = palette();
        pal.setColor(QPalette::Button, color);
        setPalette(pal);
    }
};

AGZ_EDITOR_END
