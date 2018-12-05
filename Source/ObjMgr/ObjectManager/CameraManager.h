#pragma once

#include "../Common.h"

AGZ_NS_BEG(ObjMgr)

using CameraCreator = ObjectCreator<Atrc::Camera>;
using CameraManager = ObjectManager<Atrc::Camera>;

// film.width  = uint32_t
// film.height = uint32_t
// sensor.width    = Real
// sensor.height   = Real or null
// sensor.distance = Real
// pinholePos = Vec3
// lookAt     = Vec3
// up         = Vec3
class PinholeCameraCreator : public CameraCreator, public AGZ::Singleton<PinholeCameraCreator>
{
public:

    Str8 GetName() const override { return "Pinhole"; }

    Atrc::Camera *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

// film.width         = uint32_t
// film.height        = uint32_t
// sensor.width       = Real
// sensor.height      = Real or null
// sensor.distance    = Real
// lens.radius        = Real
// lens.focalDistance = Real or lens.focalPlaneDistance = Real
// lensPos            = Vec3
// lookAt             = Vec3
// up                 = Vec3
class ThinLensCameraCreator : public CameraCreator, public AGZ::Singleton<ThinLensCameraCreator>
{
public:

    Str8 GetName() const override { return "ThinLens"; }

    Atrc::Camera *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

AGZ_NS_END(ObjMgr)
