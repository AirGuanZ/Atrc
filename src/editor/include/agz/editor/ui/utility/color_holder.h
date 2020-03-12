#pragma once

#include <QColorDialog>
#include <QLabel>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class ColorHolder : public QLabel
{
    Q_OBJECT

public:

    explicit ColorHolder(const Spectrum &init_color, QWidget *parent = nullptr)
        : QLabel(parent)
    {
        setFixedHeight(height());
        setScaledContents(true);

        color_ = QColor::fromRgbF(init_color.r, init_color.g, init_color.b);
        update_label_color();
    }

    Spectrum get_color() const noexcept
    {
        return qcolor_to_spectrum(color_);
    }

    const QColor &get_qcolor() const noexcept
    {
        return color_;
    }

    void set_color(const Spectrum &c)
    {
        color_ = QColor::fromRgbF(c.r, c.g, c.b);
        update_label_color();
    }

signals:

    void change_color(const Spectrum &color);

protected:

    void mousePressEvent(QMouseEvent *event) override
    {
        pressed_ = true;
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if(!pressed_)
            return;
        pressed_ = false;

        QColor new_color = QColorDialog::getColor(color_, this);
        if(new_color.isValid())
        {
            color_ = new_color;
            update_label_color();
            emit change_color(get_color());
        }
    }

    void leaveEvent(QEvent *event) override
    {
        pressed_ = false;
    }

private:

    void update_label_color()
    {
        QImage img(1, 1, QImage::Format::Format_RGB888);
        img.setPixelColor(0, 0, color_);

        QPixmap pixmap;
        pixmap.convertFromImage(img);

        setPixmap(pixmap);
    }

    QColor color_;

    bool pressed_ = false;
};

AGZ_EDITOR_END
