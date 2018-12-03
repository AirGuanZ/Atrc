#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class VerticalFlipper : public PostProcessStage
{
public:

    void Process(RenderTarget &img) const override;
};

AGZ_NS_END(Atrc)
