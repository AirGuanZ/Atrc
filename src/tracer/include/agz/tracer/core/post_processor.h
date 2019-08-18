﻿#pragma once

#include <agz/tracer/core/film.h>
#include <agz/tracer_utility/object.h>

AGZ_TRACER_BEGIN

/**
 * @brief 图像后处理接口
 */
class PostProcessor : public obj::Object
{
public:

    using Object::Object;

    virtual void process(texture::texture2d_t<Spectrum> &image, GBuffer &gbuffer) = 0;
};

AGZT_INTERFACE(PostProcessor)

AGZ_TRACER_END
