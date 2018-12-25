#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

Context::Context(const ConfigGroup &root, const Str8 &configFilename)
    : root_(root), configPath_(Path8(configFilename, true).ToDirectory())
{
    if(auto pNode = root.Find("workspace"))
    {
        auto &ws = pNode->AsValue();
        if(ws.StartsWith("@"))
            workspace_ = configPath_ + Path8(ws.Slice(1), false);
        else
            workspace_ = Path8(pNode->AsValue(), false);
        //throw MgrErr(Str8(ws.Slice(1)));
    }
    if(workspace_.HasFilename())
        throw MgrErr("Invalid workspace directory");
}

Str8 Context::GetPathInWorkspace(const Str8 &subFilename) const
{
    if(subFilename.StartsWith("$"))
        return subFilename.Slice(1);
    if(subFilename.StartsWith("@"))
        return (configPath_ + Path8(subFilename.Slice(1), true)).ToStr();
    Path8 p(subFilename, true);
    if(p.IsAbsolute())
        return subFilename;
    return (workspace_ + p).ToStr();
}

} // namespace Atrc::Mgr
