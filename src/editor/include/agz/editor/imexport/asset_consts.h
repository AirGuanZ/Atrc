#pragma once

#include <array>
#include <tuple>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

struct AssetVersion
{
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t fix   = 1;

    bool operator<(const AssetVersion &rhs) const noexcept
    {
        return std::tie(major, minor, fix) < std::tie(rhs.major, minor, fix);
    }
};

constexpr AssetVersion make_asset_version(
    uint32_t major, uint32_t minor, uint32_t fix) noexcept
{
    return { major, minor, fix };
}

inline const std::array<unsigned char, 8> &get_asset_head_magic() noexcept
{
    static std::array<unsigned char, 8> ret =
        { 0x12, 0x38, 0x21, 0xa7, 0x9c, 0xdb, 0x5f, 0x66 };
    return ret;
}

AGZ_EDITOR_END
