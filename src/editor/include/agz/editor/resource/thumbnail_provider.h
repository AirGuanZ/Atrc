#pragma once

#include <QPixmap>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

/**
 * @brief compute the thumbnail for a specific resource asynchronously
 */
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

    /**
     * @brief a newer version of thumbnail is generated
     */
    void update_thumbnail(QPixmap new_thumbnail);
};

/**
 * @brief empty thumbnail generator
 *
 * the generated thumbnail has unspecific pixel values
 */
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

/**
 * @brief return a fixed thumbnail provided to constructor
 */
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
