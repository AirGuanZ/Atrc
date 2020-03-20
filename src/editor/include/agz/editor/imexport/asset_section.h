#pragma once

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

enum class AssetSectionType : uint8_t
{
    MaterialPool  = 0,
    MediumPool    = 1,
    GeometryPool  = 2,
    Texture2DPool = 3,
    Texture3DPool = 4,

    Entities      = 6,
    EnvirLight    = 7,

    GlobalSettings = 8,
    PostProcessors = 9,
    PreviewWindow  = 10,

    Renderer = 11,

};

AGZ_EDITOR_END
