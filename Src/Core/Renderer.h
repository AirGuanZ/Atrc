#pragma once

#include <Utils.h>

#include "../Common.h"

AGZ_NS_BEG(Atrc)

ATRC_INTERFACE Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render() = 0;
};

AGZ_NS_END(Atrc)
