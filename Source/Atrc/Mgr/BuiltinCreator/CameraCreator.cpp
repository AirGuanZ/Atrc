#include <Atrc/Lib/Camera/PinholeCamera.h>
#include <Atrc/Mgr/BuiltinCreator/CameraCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinCameraCreators(Context &context)
{
    static const PinholeCameraCreator pinholeCameraCreator;
    context.AddCreator(&pinholeCameraCreator);
}

Camera *PinholeCameraCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto filmSize = Parser::ParseVec2i(context.Root()["film.size"]);
        if(filmSize.x <= 0 || filmSize.y <= 0)
            throw MgrErr("Invalid film size value");
        
        auto sensorRectSize = Parser::ParseVec2(group["sensorRectSize"]);
        if(sensorRectSize.x <= 0 || sensorRectSize.y <= 0)
            throw MgrErr("Invalid sensor rect size value");
        
        auto sensorDistance = group["sensorDistance"].Parse<Real>();
        if(sensorDistance <= 0)
            throw MgrErr("Invalid sensorDistance value");
        
        auto pinholePos = Parser::ParseVec3(group["pinholePos"]);
        auto lookAt     = Parser::ParseVec3(group["lookAt"]);
        auto up         = Parser::ParseVec3(group["up"]);

        if(up == Vec3(Real(0)))
            throw MgrErr("Up vector must be non-zero");
        
        return arena.Create<PinholeCamera>(
            filmSize.x, filmSize.y, sensorRectSize,
            sensorDistance, pinholePos, lookAt, up);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating pinhole camera: " + group.ToString())
}

} // namespace Atrc::Mgr
