#include <Atrc/Core/Camera/PinholeCamera.h>
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
    AGZ_HIERARCHY_TRY
    {
        auto filmSize = Parser::ParseVec2i(context.Root()["film.size"]);
        if(filmSize.x <= 0 || filmSize.y <= 0)
            throw AGZ::HierarchyException("Invalid film size value");

        Vec2 sensorRectSize;
        if(auto widthNode = group.Find("sensorWidth"))
        {
            sensorRectSize.x = widthNode->Parse<Real>();
            sensorRectSize.y = sensorRectSize.x / filmSize.x * filmSize.y;
        }
        else
            sensorRectSize = Parser::ParseVec2(group["sensorRectSize"]);

        if(sensorRectSize.x <= 0 || sensorRectSize.y <= 0)
            throw AGZ::HierarchyException("Invalid sensor rect size value");
        
        auto sensorDistance = group["sensorDistance"].Parse<Real>();

        if(sensorDistance <= 0)
            throw AGZ::HierarchyException("Invalid sensorDistance value");
        
        auto pinholePos = Parser::ParseVec3(group["pos"]);
        auto lookAt     = Parser::ParseVec3(group["lookAt"]);
        auto up         = Parser::ParseVec3(group["up"]);

        if(up == Vec3(Real(0)))
            throw AGZ::HierarchyException("Up vector must be non-zero");
        
        return arena.Create<PinholeCamera>(
            filmSize.x, filmSize.y, sensorRectSize,
            sensorDistance, pinholePos, lookAt, up);
    }
    AGZ_HIERARCHY_WRAP("In creating pinhole camera: " + group.ToString())
}

} // namespace Atrc::Mgr
