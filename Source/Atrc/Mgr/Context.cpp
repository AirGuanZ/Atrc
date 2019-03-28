#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

Context::Context(const ConfigGroup &root, std::string_view configPath)
    : root_(root), configPath_(std::filesystem::path(configPath))
{
    if(auto pNode = root.Find("workspace"))
    {
        auto &ws = pNode->AsValue();
        if(AGZ::StartsWith(ws, "@"))
            workspace_ = configPath_ / std::filesystem::path(ws.substr(1));
        else
            workspace_ = std::filesystem::path(pNode->AsValue());
    }
    else
        throw AGZ::HierarchyException("'workspace' is undefined");
}

std::string Context::GetPathInWorkspace(std::string_view subFilename) const
{
    if(AGZ::StartsWith(subFilename, "$"))
        return std::string(subFilename.substr(1));
    if(AGZ::StartsWith(subFilename, "@"))
        return (configPath_ / std::filesystem::path(subFilename.substr(1))).string();
    std::filesystem::path p(subFilename);
    if(p.is_absolute())
        return std::string(subFilename);
    return (workspace_ / p).string();
}

} // namespace Atrc::Mgr
