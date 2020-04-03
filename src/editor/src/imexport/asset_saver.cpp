#include <agz/editor/imexport/asset_consts.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/imexport/asset_version.h>

AGZ_EDITOR_BEGIN

AssetSaver::AssetSaver(std::ostream &fout)
    : fout_(fout)
{
    auto &m = get_asset_head_magic();
    write_raw(m.data(), m.size());
    write(versions::V_latest);
}

AGZ_EDITOR_END
