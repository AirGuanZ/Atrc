#pragma once

#include <AGZUtils/Config/Config.h>
#include <Atrc/Core/Core/Common.h>

namespace Atrc
{

DEFINE_ATRC_EXCEPTION(ConfigConvertException);
    
std::string Vec2ToCS(const Vec2 &vec);

std::string Vec3ToCS(const Vec3 &vec);

std::string Vec3fToCS(const AGZ::Math::Vec3f &vec);

std::string BoolToCS(bool value);

Vec2 Node2Vec2(const AGZ::ConfigNode &node);

AGZ::Math::Vec3f Node2Vec3f(const AGZ::ConfigNode &node);

bool Node2Bool(const AGZ::ConfigNode &node);

} // namespace Atrc
