#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

/*
    type = Pinhole

    (global) film.size = Vec2i
    
    sensorRectSize = Vec2
    sensorDistance = Real
    pinholePos     = Vec3
    lookAt         = Vec3
    up             = Vec3
*/
class PinholeCameraCreator : public Creator<Camera>
{
public:

    Str8 GetTypeName() const override { return "Pinhole"; }

    const Camera *Create(const ConfigGroup &group, Context &context, Arena &arena) override;
};

} // namespace Atrc::Mgr
