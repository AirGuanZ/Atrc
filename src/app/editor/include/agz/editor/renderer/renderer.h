#pragma once

#include <QObject>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

/**
 * @brief interactive path tracer
 *
 * 除构造和析构，若非特殊说明，所有方法都是线程安全的
 */
class Renderer : public QObject, public misc::uncopyable_t
{
    Q_OBJECT

public:

    virtual ~Renderer() = default;

    virtual void start() = 0;

    virtual Image2D<math::color3b> get_image() const = 0;

signals:

    void can_get_img();
};

AGZ_EDITOR_END
