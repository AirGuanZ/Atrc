#pragma once

#include <QObject>

#include <agz/editor/renderer/framebuffer.h>

AGZ_EDITOR_BEGIN

/**
 * @brief interactive path tracer
 *
 * all public methods (exclude destructor) must be thread safe
 */
class Renderer : public QObject, public misc::uncopyable_t
{
public:

    virtual ~Renderer() = default;

    /**
     * @brief called before start rendering
     *
     * @return a quickly rendered image for previewing
     */
    virtual Image2D<Spectrum> start() = 0;

    /**
     * @brief get current rendered image
     */
    virtual Image2D<Spectrum> get_image() const = 0;
};

AGZ_EDITOR_END
