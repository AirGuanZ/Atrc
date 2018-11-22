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

    // Medium��sp�Դ�
    Spectrum L_right(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const;

    Spectrum L2_right(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const;

    Spectrum E1_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const;

    Spectrum E2_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd) const;

    Spectrum S_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const;

    Spectrum Ls_right(const Scene &scene, const MediumPoint &mp, int depth, AGZ::ObjArena<> &arena) const;

    // r���е�sp��������medium����û���У�����global medium
    // D2(x \leftarrow Phi)��Ӧr = x + t\Phi
    Spectrum D2_left(const Scene &scene, const Ray &r, int depth, AGZ::ObjArena<> &arena) const;

    // ��֪r��ȥ��������sp���������D2_left���
    Spectrum D2_left(const Scene &scene, const Ray &r, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const;

    Spectrum L_left(const Scene &scene, const Ray &r, int depth, AGZ::ObjArena<> &arena) const;

    int maxDepth_;
};

AGZ_NS_END(Atrc)
