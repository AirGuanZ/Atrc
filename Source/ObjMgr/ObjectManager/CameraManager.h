#pragma once

#include "../Common.h"

AGZ_NS_BEG(ObjMgr)

using CameraCreator = ObjectCreator<Atrc::Camera>;
using CameraManager = ObjectManager<Atrc::Camera>;

class PerspectiveCameraCreator : public CameraCreator, public AGZ::Singleton<PerspectiveCameraCreator>
{
public:

    Str8 GetName() const override { return "Perspective"; }

    Atrc::Camera *Create(const ConfigGroup &params, ObjArena<> &arena) const override;
};

AGZ_NS_END(ObjMgr)
