#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

Vec2 CommonTransformCamera::Film2Sensor(const Vec2 &rasterPos) const
{
    return Vec2(
        wS_ * (rasterPos.x / wI_) - halfWS_,
        halfHS_ - hS_ * (rasterPos.y / hI_));
}

Vec2 CommonTransformCamera::Sensor2Film(const Vec2 &sensorPos) const
{
    return Vec2(
        (sensorPos.x / wS_ + Real(0.5)) * wI_,
        (Real(0.5) - sensorPos.y / hS_) * hI_);
}

Vec3 CommonTransformCamera::LocalPoint2World(const Vec3 &local) const
{
    return R_ * local + T_;
}

Vec3 CommonTransformCamera::WorldPoint2Local(const Vec3 &world) const
{
    return invR_ * (world - T_);
}

Vec3 CommonTransformCamera::LocalVector2World(const Vec3 &local) const
{
    return R_ * local;
}

Vec3 CommonTransformCamera::WorldVector2Local(const Vec3 &world) const
{
    return invR_ * world;
}

CommonTransformCamera::CommonTransformCamera(
        const Vec2 &filmSize, const Vec2 &sensorRectSize,
        const Vec3 &centrePos, const Vec3 &lookAt, const Vec3 &u)
    : Camera(filmSize.x, filmSize.y),
      wS_(sensorRectSize.x), hS_(sensorRectSize.y),
      halfWS_(sensorRectSize.x / 2), halfHS_(sensorRectSize.y / 2),
      T_(centrePos)
{
    Vec3 ex = (lookAt - centrePos).Normalize();
    Vec3 ey = Cross(u, ex).Normalize();
    Vec3 ez = Cross(ex, ey);
    R_    = Mat3::FromCols(ex, ey, ez);
    invR_ = R_.Transpose();
}

AGZ_NS_END(Atrc)
