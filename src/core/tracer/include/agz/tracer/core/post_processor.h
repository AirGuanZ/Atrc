#pragma once

#include <agz/tracer/core/render_target.h>

AGZ_TRACER_BEGIN

/**
 * @brief post processor interface
 */
class PostProcessor
{
public:

    virtual ~PostProcessor() = default;

    virtual void process(RenderTarget &render_target) = 0;
};

AGZ_TRACER_END
