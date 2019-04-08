#pragma once

#include <Atrc/Editor/EditorCore.h>

namespace Atrc::Editor
{

class LauncherExporter
{
public:

    std::string Export(EditorData *data) const;
};

}; // namespace Atrc::Editor
