#ifdef USE_EMBREE

#include <agz/tracer/utility/embree.h>

AGZ_TRACER_BEGIN

namespace
{
    RTCDevice g_device = nullptr;
}

void init_embree_device()
{
    assert(!g_device);
    g_device = rtcNewDevice(nullptr);
}

void destroy_embree_device()
{
    assert(g_device);
    rtcReleaseDevice(g_device);
}

RTCDevice embree_device()
{
    assert(g_device);
    return g_device;
}

AGZ_TRACER_END

#endif // #ifndef USE_EMBREE
