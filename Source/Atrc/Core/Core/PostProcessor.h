#pragma once

#include <list>

#include <Atrc/Core/Core/Common.h>

namespace Atrc
{
    
class PostProcessor
{
public:

    virtual ~PostProcessor() = default;

    virtual void Process(Image *image) const noexcept = 0;
};

class PostProcessorChain
{
    std::list<const PostProcessor*> processors_;

public:

    void AddFront(const PostProcessor *processor);

    void AddBack(const PostProcessor *processor);

    void Process(Image *image) const noexcept;
};

} // namespace Atrc
