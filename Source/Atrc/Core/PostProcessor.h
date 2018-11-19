#pragma once

#include <vector>

#include <Atrc/Core/Common.h>

AGZ_NS_BEG(Atrc)

class PostProcessStage
{
public:

    virtual ~PostProcessStage() = default;

    virtual void Process(RenderTarget &img) const = 0;
};

class PostProcessor
{
    std::vector<const PostProcessStage*> stages_;

public:

    void AddStage(const PostProcessStage *stage);

    void Process(RenderTarget &img) const;
};

AGZ_NS_END(Atrc)
