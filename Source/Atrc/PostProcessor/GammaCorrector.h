#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class GammaCorrector : public PostProcessStage
{
    float gamma_;

public:

    explicit GammaCorrector(float gamma);

    void Process(RenderTarget &img) const override;
};

AGZ_NS_END(Atrc)
