#include <Atrc/Core/PostProcessor.h>

AGZ_NS_BEG(Atrc)

void PostProcessor::AddStage(const PostProcessStage *stage)
{
    AGZ_ASSERT(stage);
    stages_.push_back(stage);
}

void PostProcessor::Process(RenderTarget &img) const
{
    for(auto stage : stages_)
        stage->Process(img);
}

AGZ_NS_END(Atrc)
