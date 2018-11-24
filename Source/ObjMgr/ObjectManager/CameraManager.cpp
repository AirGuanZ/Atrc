#include "CameraManager.h"
#include "ObjMgr/ParamParser.h"

AGZ_NS_BEG(ObjMgr)

Atrc::Camera *PerspectiveCameraCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto eye = ParamParser::ParseVec3(params["eye"]);
    auto dst = ParamParser::ParseVec3(params["dst"]);
    auto up  = ParamParser::ParseVec3(params["up"]);
    auto dir = dst - eye;

    auto aspectRatio = params["aspectRatio"].AsValue().Parse<Atrc::Real>();

    Atrc::Rad rad;
    auto &angle = params["FOVz"].AsArray();
    if(angle.Size() != 1)
        throw SceneInitializationException("Scene: invalid angle form for FOVy");
    if(angle.GetTag() == "Rad")
        rad = Atrc::Rad(angle[0].AsValue().Parse<Atrc::Real>());
    else if(angle.GetTag() == "Deg")
        rad = Atrc::Deg(angle[0].AsValue().Parse<Atrc::Real>());
    else
        throw SceneInitializationException("Scene: invalid angle form for FOVy");

    return arena.Create<Atrc::PerspectiveCamera>(eye, dir, up, rad, aspectRatio);
}

AGZ_NS_END(ObjMgr)
