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
    // TODO：Qe现在是1，这样下去是不行的

    Real y = rasterPos.y / hI_;

    Real theta = -2 * PI * rasterPos.x / wI_;
    Real phi   = PI / 2 - PI * y;
    Real cosPhi = Cos(phi);

    auto dir = Vec3(cosPhi * Cos(theta), cosPhi * Sin(theta), Sin(phi)).Normalize();

    return {
        Ray(LocalPoint2World(Vec3(Real(0))), LocalVector2World(dir), EPS),
        Spectrum(1),
        Real(1)
    };
}

AGZ_NS_END(Atrc)
