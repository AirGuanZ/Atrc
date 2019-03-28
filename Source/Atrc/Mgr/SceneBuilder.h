#pragma once

#include <Atrc/Core/Core/Scene.h>
#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

class SceneBuilder
{
public:

    static Scene Build(const ConfigGroup &root, Context &context);
};

} // namespace Atrc::Mgr
