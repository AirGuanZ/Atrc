#pragma once

#include <Atrc/Atrc/ResourceInterface/ResourceInstance.h>

class BoxCore
{
    float sidelen_;

public:

    static const char *GetTypeName() noexcept
    {
        return "Box";
    }

    static bool IsMultiline() noexcept { return false; }

    explicit BoxCore(ResourceCreateContext&);

    void Display(ResourceCreateContext &ctx);
};
