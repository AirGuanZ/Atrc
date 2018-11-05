#include <Atrc/Integrator/VolumetricPathTracer.h>

AGZ_NS_BEG(Atrc)

VolumetricPathTracer::VolumetricPathTracer(int maxDepth)
	: maxDepth_(maxDepth)
{

}

Spectrum VolumetricPathTracer::GetRadiance(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const
{

}

AGZ_NS_END(Atrc)
