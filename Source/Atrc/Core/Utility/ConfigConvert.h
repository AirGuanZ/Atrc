#pragma once

#include <AGZUtils/Config/Config.h>
#include <Atrc/Core/Core/Common.h>

namespace Atrc
{

DEFINE_ATRC_EXCEPTION(ConfigConvertException);
    
std::string Vec2ToConfigStr(const Vec2 &vec);

Vec2 Node2Vec2(const AGZ::ConfigNode &node);

} // namespace Atrc
