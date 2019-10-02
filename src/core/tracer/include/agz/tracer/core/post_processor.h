#pragma once

#include <agz/tracer/core/film.h>

AGZ_TRACER_BEGIN

/**
 * @brief 图像后处理接口
 */
class PostProcessor
{
public:

    virtual ~PostProcessor() = default;

    /**
     * @brief 处理图像和gbuffer
     */
    virtual void process(texture::texture2d_t<Spectrum> &image, GBuffer &gbuffer) = 0;
};

AGZ_TRACER_END
