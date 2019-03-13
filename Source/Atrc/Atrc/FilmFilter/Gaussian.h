#pragma once

#include <Atrc/Atrc/ResourceInterface/ResourceInstance.h>

class GaussianCore
{
    float radius_;
    float alpha_;

public:

    static const char *GetTypeName() noexcept { return "Gaussian"; }

    static bool IsMultiline() noexcept { return true; }

    explicit GaussianCore(const ResourceCreateContext&);

    void Display(ResourceCreateContext &ctx);
};
