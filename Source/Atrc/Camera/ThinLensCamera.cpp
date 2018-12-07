#include <Atrc/Camera/ThinLensCamera.h>

AGZ_NS_BEG(Atrc)

Real ThinLensCamera::FocalPlaneDistance2FocalDistance(Real sensorLensDistance, Real focalPlaneDistance)
{
    AGZ_ASSERT(focalPlaneDistance > 0 && sensorLensDistance < focalPlaneDistance);
    return 1 / (1 / sensorLensDistance - 1 / focalPlaneDistance);
}

ThinLensCamera::ThinLensCamera(
    const Vec2 &filmSize, const Vec2 &sensorRectSize,
    const Vec3 &lensPos, const Vec3 &lookAt, const Vec3 &u,
    Real sensorLensDistance, Real lensRadius, Real focalDistance)
    : CommonTransformCamera(filmSize, sensorRectSize, lensPos, lookAt, u),
      L_(sensorLensDistance), lensRadius_(lensRadius)
{
    AGZ_ASSERT(sensorLensDistance < focalDistance && 0 < sensorLensDistance);
    areaLens_ = PI * lensRadius * lensRadius;
    focalPlaneDistance_ = 1 / (1 / L_ - 1 / focalDistance);
}

/*
    1. 计算无透镜时的射线与焦平面的交点p
    2. 在透镜上采样一点x，取x + te_{xp}
*/
CameraRay ThinLensCamera::GetRay(const Vec2 &rasterPos) const
{
    auto sensorPos = Film2Sensor(rasterPos);
    auto localDir = Vec3(L_, sensorPos.x, -sensorPos.y).Normalize();

    if(!lensRadius_)
    {
        return {
            Ray(LocalPoint2World(Vec3(Real(0))),
                LocalVector2World(localDir).Normalize(),
                EPS),
            Spectrum(1.0f), Real(1)
        };
    }

    Real t = focalPlaneDistance_ / localDir.x;
    auto p = t * localDir;

    Real u0 = Rand(), u1 = Rand();
    auto xSample = AGZ::Math::DistributionTransform::
        UniformOnUnitDisk<Real>::Transform({ u0, u1 });
    Vec3 x(Real(0), lensRadius_ * xSample.sample);

    Real cosFactor  = localDir.x; // Dot(UNIT_X, localDir)
    Real cosFactor3 = cosFactor * cosFactor * cosFactor;
    Real pdf = xSample.pdf / (lensRadius_ * lensRadius_) * (L_ * L_) / cosFactor3;
    Real qe  = L_ * L_ / (areaLens_ * cosFactor3);

    return {
        Ray(
            LocalPoint2World(x),
            LocalVector2World((p - x)).Normalize(),
            EPS),
        Spectrum(float(qe)),
        pdf
    };
}

AGZ_NS_END(Atrc)
