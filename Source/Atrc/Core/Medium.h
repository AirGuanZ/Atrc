#pragma once

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

class Medium
{
public:

    virtual ~Medium() = default;

    virtual Spectrum Tr(const Ray &r) const = 0;
};

struct MediumInterface
{
    const Medium *in = nullptr, *out = nullptr;
};

AGZ_NS_END(Atrc)
