#include <Atrc/Lib/Entity/GeometricEntity.h>
#include <Atrc/Mgr/BuiltinCreator/EntityCreator.h>

namespace Atrc::Mgr
{

const Entity *GeometricEntityCreator::Create(const ConfigGroup &group, Context &context, Arena &arena)
{
    ATRC_MGR_TRY
    {
        auto geometry = context.Create<Geometry>(group["geometry"]);
        auto material = context.Create<Material>(group["material"]);
        return arena.Create<GeometricEntity>(geometry, material);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating geometric entity: " + group.ToString())
}

} // namespace Atrc::Mgr
