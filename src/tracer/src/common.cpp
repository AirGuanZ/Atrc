#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

real EPS = real(3e-4);

void set_eps(real new_eps) noexcept
{
    EPS = new_eps;
}

AGZ_TRACER_END
