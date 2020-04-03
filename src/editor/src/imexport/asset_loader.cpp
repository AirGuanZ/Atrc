#include <agz/editor/imexport/asset_consts.h>
#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_version.h>

AGZ_EDITOR_BEGIN

namespace
{
    bool try_to_load(
        std::istream &fin, size_t bytes, void *output)
    {
        const auto old_p = fin.tellg();
        fin.read(reinterpret_cast<char *>(output), bytes);

        if(!fin)
        {
            fin.clear();
            fin.seekg(old_p);
            return false;
        }

        return true;
    }
}

AssetLoader::AssetLoader(std::istream &fin)
    : fin_(fin)
{
    // load magic number & version number

    const auto &std_magic = get_asset_head_magic();

    rm_rcv_t<decltype(std_magic)> magic = { 0 };
    if(!try_to_load(fin, magic.size(), magic.data()) || magic != std_magic)
    {
        fin.seekg(0);
        version_ = versions::V0000_0000_0000;
    }
    else if(!try_to_load(fin, sizeof(version_), &version_))
        version_ = versions::V0000_0000_0000;
}

AGZ_EDITOR_END
