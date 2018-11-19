#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

// See https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
class ACESFilm : public PostProcessStage
{
public:

    void Process(RenderTarget &img) const override;
};

AGZ_NS_END(Atrc)
