#pragma once

#include <QPixmap>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class ResourceThumbnailProvider : public QObject
{
    Q_OBJECT

public:

    virtual ~ResourceThumbnailProvider() = default;

    /**
     * @brief get the initial thumbnail
     */
    virtual QPixmap start() = 0;

signals:

    void update_thumbnail(QPixmap new_thumbnail);
};

class EmptyResourceThumbnailProvider : public ResourceThumbnailProvider
{
    int width_, height_;

public:

    EmptyResourceThumbnailProvider(int width, int height)
        : width_(width), height_(height)
    {
        
    }

    QPixmap start() override
    {
        return QPixmap(width_, height_);
    }
};

class FixedResourceThumbnailProvider : public ResourceThumbnailProvider
{
    QPixmap pixmap_;

public:

    explicit FixedResourceThumbnailProvider(QPixmap pixmap)
        : pixmap_(std::move(pixmap))
    {
        
    }

    QPixmap start() override
    {
        return std::move(pixmap_);
    }
};

AGZ_EDITOR_END
