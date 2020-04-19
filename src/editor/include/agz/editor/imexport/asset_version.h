#pragma once

#include <agz/editor/imexport/asset_consts.h>

AGZ_EDITOR_BEGIN

namespace versions
{

    constexpr AssetVersion V0000_0000_0000 = make_asset_version(0, 0, 0);
    constexpr AssetVersion V2020_0330_1140 = make_asset_version(0, 1, 0);
    constexpr AssetVersion V2020_0404_1413 = make_asset_version(0, 1, 1);
    constexpr AssetVersion V2020_0418_2358 = make_asset_version(1, 0, 0);

    constexpr AssetVersion V_latest = V2020_0418_2358;

} // namespace versions

AGZ_EDITOR_END
