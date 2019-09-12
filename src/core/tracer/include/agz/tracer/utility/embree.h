#pragma once

#ifdef USE_EMBREE

#include <embree3/rtcore_device.h>

#include <agz/common/common.h>

AGZ_TRACER_BEGIN

void init_embree_device();

void destroy_embree_device();

RTCDevice embree_device();

AGZ_TRACER_END

#endif // #ifdef USE_EMBREE
