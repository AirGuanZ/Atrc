#include <filesystem>

#include <Atrc/Mgr/Common.h>

namespace Atrc::Mgr
{

std::string GetCacheFilename(std::string_view filename)
{
    std::filesystem::path parent("./.agz.cache/");
    for(auto &sec : relative(std::filesystem::path(filename)))
    {
        if(sec == "..")
            parent.append("__lastlast");
        else if(sec != ".")
            parent /= sec;
    }
    return parent.string();
}

} // namespace Atrc::Mgr
