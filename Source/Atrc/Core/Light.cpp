#include <Atrc/Core/Light.h>

AGZ_NS_BEG(Atrc)

bool Light::IsDelta() const
{
    return IsDeltaPosition() || IsDeltaDirection();
}

AGZ_NS_END(Atrc)
