#pragma once

#include <Utils.h>

AGZ_NS_BEG(Atrc)

class Renderer
{
public:

    virtual ~Renderer() = default;

    virtual void Render() = 0;
};

AGZ_NS_END(Atrc)
