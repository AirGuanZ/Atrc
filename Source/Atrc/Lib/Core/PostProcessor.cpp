#include <Atrc/Lib/Core/PostProcessor.h>

namespace Atrc
{
    
void PostProcessorChain::AddFront(const PostProcessor *processor)
{
    AGZ_ASSERT(processor);
    processors_.push_front(processor);
}

void PostProcessorChain::AddBack(const PostProcessor *processor)
{
    AGZ_ASSERT(processor);
    processors_.push_back(processor);
}

void PostProcessorChain::Process(Image *image) const noexcept
{
    AGZ_ASSERT(image);
    for(auto p : processors_)
        p->Process(image);
}

} // namespace Atrc
