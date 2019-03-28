#pragma once

#include <AGZUtils/Utils/Config.h>
#include <Atrc/Core/Core/Common.h>

namespace Atrc::Mgr
{

using AGZ::Config;
using AGZ::ConfigArray;
using AGZ::ConfigGroup;
using AGZ::ConfigNode;
using AGZ::ConfigValue;

std::string GetCacheFilename(std::string_view filename);

} // namespace Atrc::Mgr
