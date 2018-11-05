#include <Atrc/Core/Light.h>

AGZ_NS_BEG(Atrc)

void Light::PreprocessScene(const Scene &scene)
{
    // do nothing
}

bool Light::IsDelta() const
{
    return IsDeltaPosition() || IsDeltaDirection();
}

bool Light::NonareaLeIgnoreGlobalMedium() const
{
	return false;
}

AGZ_NS_END(Atrc)
