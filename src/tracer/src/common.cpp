#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

namespace
{
    real EPS_value = real(3e-4);
}

real EPS() noexcept
{
    return EPS_value;
}

void set_eps(real new_eps) noexcept
{
    EPS_value = new_eps;
}

AGZ_TRACER_END
