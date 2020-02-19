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

/*
class ColorHolder1 : public QWidget
{
    Q_OBJECT

public:

    explicit ColorHolder1(const Spectrum &init_color, QWidget *parent = nullptr)
        : QWidget(parent)
    {
        button_ = new QPushButton("change", this);
        label_  = new QLabel(this);

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->addWidget(button_);
        layout->addWidget(label_);

        color_ = QColor::fromRgbF(init_color.r, init_color.g, init_color.b);
        set_displayed_color(color_);

        connect(button_, &QPushButton::clicked, [=]
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

    const QColor &get_qcolor() const noexcept
    {
        return color_;
    }

signals:

    void change_color(const Spectrum &color);

private:

    QPushButton *button_;
    QLabel      *label_;

    QColor color_;

    void set_displayed_color(const QColor &color)
    {
        label_->setAutoFillBackground(true);
        QPalette pal = label_->palette();
        pal.setColor(QPalette::Window, color);
        label_->setPalette(pal);
    }
};

class ColorHolder2 : public QPushButton
{
    Q_OBJECT

public:

    explicit ColorHolder2(const Spectrum &init_color, QWidget *parent = nullptr)
        : QPushButton(parent)
    {
        setFlat(true);

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

    const QColor &get_qcolor() const noexcept
    {
        return color_;
    }

signals:

    void change_color(const Spectrum &new_color);

private:

    QColor color_;

    void set_displayed_color(const QColor &color)
    {
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Button, color);
        //pal.setColor(backgroundRole(), color);
        //pal.setColor(QPalette::Window, color);
        setPalette(pal);
    }
};
*/

AGZ_EDITOR_END
