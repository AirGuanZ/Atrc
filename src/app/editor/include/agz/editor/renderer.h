#pragma once

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

/**
 * @brief interactive path tracer
 *
 * 除构造和析构，若非特殊说明，所有方法都是线程安全的
 */
class Renderer : public misc::uncopyable_t
{
public:

    virtual ~Renderer() = default;

    virtual Image2D<math::color3b> get_image() const = 0;
};

AGZ_EDITOR_END
