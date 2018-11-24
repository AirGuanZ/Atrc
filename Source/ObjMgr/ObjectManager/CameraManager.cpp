#include "CameraManager.h"
#include "ObjMgr/ParamParser.h"

AGZ_NS_BEG(ObjMgr)

Atrc::Camera *PerspectiveCameraCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto eye = ParamParser::ParseVec3(params["eye"]);
    auto dst = ParamParser::ParseVec3(params["dst"]);
    auto up  = ParamParser::ParseVec3(params["up"]);
    auto rad = ParamParser::ParseAngle(params["FOVz"]);
    auto aspectRatio = params["aspectRatio"].AsValue().Parse<Atrc::Real>();
    auto dir = dst - eye;

    return arena.Create<Atrc::PerspectiveCamera>(eye, dir, up, rad, aspectRatio);
}

AGZ_NS_END(ObjMgr)
