#include "CameraManager.h"
#include "ObjMgr/ParamParser.h"

AGZ_NS_BEG(ObjMgr)

Atrc::Camera *PinholeCameraCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto filmWidth  = params["film.width"] .AsValue().Parse<uint32_t>();
    auto filmHeight = params["film.height"].AsValue().Parse<uint32_t>();

    auto sensorWidth  = params["sensor.width"] .AsValue().Parse<Atrc::Real>();

    Atrc::Real sensorHeight;
    auto sensorHeightNode = params.Find("sensor.height");
    if(sensorHeightNode)
        sensorHeight = sensorHeightNode->AsValue().Parse<Atrc::Real>();
    else
        sensorHeight = sensorWidth / filmWidth * filmHeight;

    auto sensorPinholeDistance = params["sensor.distance"].AsValue().Parse<Atrc::Real>();

    auto pinholePos = ParamParser::ParseVec3(params["pinholePos"]);
    auto lookAt     = ParamParser::ParseVec3(params["lookAt"]);
    auto up         = ParamParser::ParseVec3(params["up"]);

    return arena.Create<Atrc::PinholeCamera>(
        Atrc::Vec2(Atrc::Real(filmWidth), Atrc::Real(filmHeight)),
        Atrc::Vec2(sensorWidth, sensorHeight),
        sensorPinholeDistance, pinholePos, lookAt, up);
}

AGZ_NS_END(ObjMgr)
