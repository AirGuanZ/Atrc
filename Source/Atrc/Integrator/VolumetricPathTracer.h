#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

// See https://airguanz.github.io/2018/10/28/LTE-with-participating-medium.html
class VolumetricPathTracer : public Integrator
{
public:

	explicit VolumetricPathTracer(int maxDepth);

	Spectrum GetRadiance(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const override;

private:

	// Medium由sp自带
	Spectrum L_right(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const;

	Spectrum L2_right(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const;

	Spectrum E1_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const;

	Spectrum E2_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const;

	Spectrum S_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const;

	Spectrum Ls_right(const Scene &scene, const MediumPoint &mp, int depth, AGZ::ObjArena<> &arena) const;

	// r击中的sp决定它的medium，若没击中，就是global medium
	// D2(x \leftarrow Phi)对应r = x + t\Phi
	Spectrum D2_left(const Scene &scene, const Ray &r, int depth, AGZ::ObjArena<> &arena) const;

	// 已知r出去后碰到了sp，求上面的D2_left结果
	Spectrum D2_left(const Scene &scene, const Ray &r, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const;

	Spectrum L_left(const Scene &scene, const Ray &r, int depth, AGZ::ObjArena<> &arena) const;

	int maxDepth_;
};

AGZ_NS_END(Atrc)
