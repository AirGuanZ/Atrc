#pragma once

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <agz/editor/resource/thumbnail_provider.h>

AGZ_EDITOR_BEGIN

class ImageTextIcon : public QWidget
{
    Q_OBJECT

public:

    ImageTextIcon(QWidget *parent, const QFont &font, int image_size)
        : QWidget(parent), image_size_(image_size)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        image_ = new QLabel(this);
        text_ = new QLabel(this);

        text_->setFont(font);

        image_->setAlignment(Qt::AlignCenter);
        text_->setAlignment(Qt::AlignCenter);

        layout->addWidget(image_);
        layout->addWidget(text_);
    }

    void set_text(const QString &text)
    {
        QFontMetrics metrix(text_->font());
        QString clipped_text = metrix.elidedText(
            text, Qt::ElideRight, image_size_);

        text_->setText(clipped_text);
        if(clipped_text != text)
            setToolTip(text);
    }

    void set_image(Box<ResourceThumbnailProvider> thumbnail_provider)
    {
        thumbnail_provider_ = std::move(thumbnail_provider);

        connect(thumbnail_provider_.get(), &ResourceThumbnailProvider::update_thumbnail,
                this, &ImageTextIcon::set_pixmap);

        image_->setPixmap(thumbnail_provider_->start());
    }

    void set_selected(bool selected)
    {
        is_selected_ = selected;
        update_background_color();
    }

    bool is_selected() const noexcept
    {
        return is_selected_;
    }

signals:

    void clicked();

public slots:

    void set_pixmap(QPixmap pixmap)
    {
        image_->setPixmap(pixmap);
    }

protected:

    void enterEvent(QEvent *event) override
    {
        is_in_ = true;
        update_background_color();
    }

    void leaveEvent(QEvent *event) override
    {
        is_in_ = false;
        is_pressed_ = false;
        update_background_color();
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        is_pressed_ = true;
        update_background_color();
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        AGZ_SCOPE_GUARD({ update_background_color(); });

        if(!is_pressed_)
            return;
        is_pressed_ = false;
        emit clicked();
    }

private:

    void set_background_color(const QColor &color)
    {
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, color);
        setPalette(pal);
    }

    void update_background_color()
    {
        static const QColor ORDINARY         = QColor(Qt::transparent);
        static const QColor HOVERED          = QColor(75, 75, 75);
        static const QColor PRESSED          = QColor(100, 100, 100);
        static const QColor SELECTED         = QColor(100, 100, 100);
        static const QColor SELECTED_HOVERED = QColor(125, 125, 125);
        static const QColor SELECTED_PRESSED = QColor(75, 75, 75);
    
        if(is_selected_)
        {
            if(is_in_)
            {
                if(is_pressed_)
                    set_background_color(SELECTED_PRESSED);
                else
                    set_background_color(SELECTED_HOVERED);
            }
            else
                set_background_color(SELECTED);
        }
        else
        {
            if(is_in_)
            {
                if(is_pressed_)
                    set_background_color(PRESSED);
                else
                    set_background_color(HOVERED);
            }
            else
                set_background_color(ORDINARY);
        }
    }

    int image_size_;

    QLabel *image_;
    QLabel *text_;

    bool is_in_       = false;
    bool is_pressed_  = false;
    bool is_selected_ = false;

    Box<ResourceThumbnailProvider> thumbnail_provider_;
};

AGZ_EDITOR_END
