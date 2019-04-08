#include <Atrc/Editor/Exporter/Exporter.h>
#include <Atrc/Editor/Exporter/LauncherExporter.h>

namespace Atrc::Editor
{

std::string LauncherExporter::Export(EditorData *data) const
{
    AGZ_ASSERT(data);
    Exporter exp;

    AGZ_HIERARCHY_TRY

    const std::filesystem::path workspaceDir = absolute(data->workspaceDir.GetFilename());
    const std::filesystem::path scriptDir = absolute(data->scriptFilename.GetFilename()).parent_path();

    AGZ_HIERARCHY_TRY
    exp << "workspace=\"@" << relative(workspaceDir, scriptDir).string() << "\";";
    AGZ_HIERARCHY_WRAP("in exporting workspace directory")

    AGZ_HIERARCHY_TRY
    exp << "film={" << "size=(" << data->filmSize.x << ", " << data->filmSize.y << ");";
    AGZ_HIERARCHY_WRAP("in exporting film size")

    AGZ_HIERARCHY_TRY
    exp << "filter=" << data->filmFilter->GetNoneNullResource()->Export() << ";};";
    AGZ_HIERARCHY_WRAP("in exporting film filter")

    return exp.GetString();

    AGZ_HIERARCHY_WRAP("in exporting launcher script")
}

}; // namespace Atrc::Editor
