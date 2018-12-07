#include <Atrc/Camera/EnvironmentCamera.h>

AGZ_NS_BEG(Atrc)

Vec3 EnvironmentCamera::LocalPoint2World(const Vec3 &local) const
{
    return R_ * local + T_;
}

Vec3 EnvironmentCamera::LocalVector2World(const Vec3 &local) const
{
    return R_ * local;
}

EnvironmentCamera::EnvironmentCamera(
    const Vec2 &filmSize,
    const Vec3 &centrePos, const Vec3 &lookAt, const Vec3 &up)
    : Camera(filmSize.x, filmSize.y), T_(centrePos)
{
    Vec3 ex = (lookAt - centrePos).Normalize();
    Vec3 ey = Cross(up, ex).Normalize();
    Vec3 ez = Cross(ex, ey);
    R_ = Mat3::FromCols(ex, ey, ez) * Mat3::RotateZ(Deg(180));
}

CameraRay EnvironmentCamera::GetRay(const Vec2 &rasterPos) const
{
    Real theta = -2 * PI * rasterPos.x / wI_;
    Real phi   = PI / 2 - PI * rasterPos.y / hI_;
    Real cosPhi = Cos(phi);

    auto dir = Vec3(cosPhi * Cos(theta), cosPhi * Sin(theta), Sin(phi)).Normalize();

    Real theta1WI = -2 * PI * Min<uint32_t>(uint32_t(rasterPos.x), wI_ - 1);
    Real theta2WI = -2 * PI * Min<uint32_t>(uint32_t(rasterPos.x) + 1, wI_);

    Real phi1 = PI / 2 - PI * Min<uint32_t>(uint32_t(rasterPos.y), hI_ - 1) / hI_;
    Real phi2 = PI / 2 - PI * Min<uint32_t>(uint32_t(rasterPos.y) + 1, hI_) / hI_;

    Real qe = 2 * PI * PI / Abs(theta1WI - theta2WI) * (Cos(phi) / (hI_ * (Sin(phi1) - Sin(phi2))));

    return {
        Ray(LocalPoint2World(Vec3(Real(0))), LocalVector2World(dir), EPS),
        Spectrum(float(qe)), Real(1)
    };
}

AGZ_NS_END(Atrc)
