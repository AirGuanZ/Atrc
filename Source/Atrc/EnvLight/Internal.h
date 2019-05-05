#pragma once

#include <string>
#include <vector>

#include <AGZUtils/Utils/Math.h>

std::pair<int, std::vector<AGZ::Math::Vec3f>> LoadSHFromNPY(const std::string &npyFilename);
