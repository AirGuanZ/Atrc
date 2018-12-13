#include <Atrc/Lib/Geometry/Sphere.h>
#include <Atrc/Mgr/BuiltinCreator/GeometryCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

const Geometry *SphereCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena)
{
    ATRC_MGR_TRY
    {
        auto radius    = group["radius"].Parse<Real>();
        auto transform = Parser::ParseTransform(group["transform"]);
        return arena.Create<Sphere>(transform, radius);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating sphere: " + group.ToString())
}

} // namespace Atrc::Mgr
