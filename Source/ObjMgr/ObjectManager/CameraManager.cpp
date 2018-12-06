#include "CameraManager.h"
#include "ObjMgr/ParamParser.h"

using namespace Atrc;

AGZ_NS_BEG(ObjMgr)

namespace
{
    struct FilmAndSensorInfo
    {
        uint32_t filmWidth;
        uint32_t filmHeight;
        Real sensorWidth;
        Real sensorHeight;
        Real sensorDistance;
    };

    FilmAndSensorInfo ParseFilmAndSensorInfp(const ConfigGroup &params)
    {
        auto filmWidth  = params["film.width"] .AsValue().Parse<uint32_t>();
        auto filmHeight = params["film.height"].AsValue().Parse<uint32_t>();

        auto sensorWidth  = params["sensor.width"] .AsValue().Parse<Real>();

        Real sensorHeight;
        auto sensorHeightNode = params.Find("sensor.height");
        if(sensorHeightNode)
            sensorHeight = sensorHeightNode->AsValue().Parse<Real>();
        else
            sensorHeight = sensorWidth / filmWidth * filmHeight;
        
        auto sensorDistance = params["sensor.distance"].AsValue().Parse<Real>();

        if(!filmWidth || !filmHeight ||
           sensorWidth <= 0 || sensorHeight <= 0 || sensorDistance <= 0.0)
            throw SceneInitializationException("CameraCreator: invalid film/sensor attributes");
        
        return { filmWidth, filmHeight, sensorWidth, sensorHeight, sensorDistance };
    }
}

Camera *PinholeCameraCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto [fW, fH, sW, sH, sD] = ParseFilmAndSensorInfp(params);

    auto pinholePos = ParamParser::ParseVec3(params["pinholePos"]);
    auto lookAt     = ParamParser::ParseVec3(params["lookAt"]);
    auto up         = ParamParser::ParseVec3(params["up"]);

    return arena.Create<PinholeCamera>(
        Vec2(Real(fW), Real(fH)), Vec2(sW, sH),
        sD, pinholePos, lookAt, up);
}

Camera *ThinLensCameraCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto [fW, fH, sW, sH, sD] = ParseFilmAndSensorInfp(params);

    auto lensRadius = params["lens.radius"].AsValue().Parse<Real>();
    if(lensRadius < 0)
        throw SceneInitializationException("ThinLensCameraCreator: invalid lens radius value");

    Real focalDistance = -1;
    if(auto focalDistanceNode = params.Find("lens.focalDistance"))
        focalDistance = focalDistanceNode->AsValue().Parse<Real>();
    else if(auto focalPlaneDistanceNode = params.Find("lens.focalPlaneDistance"))
    {
        focalDistance = ThinLensCamera::FocalPlaneDistance2FocalDistance(
            sD, focalPlaneDistanceNode->AsValue().Parse<Real>());
    }
    
    if(focalDistance < sD)
        throw SceneInitializationException("ThinLensCameraCreator: invalid focal distance value");

    auto lensPos = ParamParser::ParseVec3(params["lensPos"]);
    auto lookAt  = ParamParser::ParseVec3(params["lookAt"]);
    auto up      = ParamParser::ParseVec3(params["up"]);

    return arena.Create<ThinLensCamera>(
        Vec2(Real(fW), Real(fH)), Vec2(sW, sH),
        lensPos, lookAt, up, sD, lensRadius, focalDistance);
}

Camera *EnvironmentCameraCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto filmWidth  = params["film.width"] .AsValue().Parse<uint32_t>();
    auto filmHeight = params["film.height"].AsValue().Parse<uint32_t>();
    if(!filmWidth || !filmHeight)
        throw SceneInitializationException("EnvironmentCameraCreator: invalid film size");
    
    auto centrePos = ParamParser::ParseVec3(params["centrePos"]);
    auto lookAt    = ParamParser::ParseVec3(params["lookAt"]);
    auto up        = ParamParser::ParseVec3(params["up"]);

    return arena.Create<Atrc::EnvironmentCamera>(
        Vec2(Real(filmWidth), Real(filmHeight)),
        centrePos, lookAt, up);
}

AGZ_NS_END(ObjMgr)
