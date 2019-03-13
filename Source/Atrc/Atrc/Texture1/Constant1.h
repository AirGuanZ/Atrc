#pragma once

#include <Atrc/Atrc/ResourceInterface/ResourceInstance.h>

class Constant1Core
{
    float minValue_, maxValue_;
    float value_;

public:

    static const char *GetTypeName() noexcept { return "Constant1"; }

    static bool IsMultiline() noexcept { return false; }

    explicit Constant1Core(ResourceCreateContext&);

    void Display(ResourceCreateContext&);

    void SetRange(float minValue, float maxValue);
};
