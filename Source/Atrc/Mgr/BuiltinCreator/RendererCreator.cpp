#include <Atrc/Core/Renderer/PathTracingRenderer.h>
#include <Atrc/Mgr/BuiltinCreator/RendererCreator.h>
#include "Atrc/Mgr/Parser.h"

namespace Atrc::Mgr
{

void RegisterBuiltinRendererCreators(Context &context)
{
    static const PathTracingRendererCreator pathTracingRendererCreator;
    context.AddCreator(&pathTracingRendererCreator);
}

Renderer *PathTracingRendererCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto integrator  = context.Create<PathTracingIntegrator>(group["integrator"]);
        int workerCount  = group["workerCount"].Parse<int>();
        int taskGridSize = group["taskGridSize"].Parse<int>();

        int epoch = 1;
        if(auto pN = group.Find("epoch"))
            epoch = pN->Parse<int>();
        if(epoch <= 0)
            throw std::runtime_error("Invalid epoch value: " + std::to_string(epoch));

        bool shuffle = false;
        if(auto pN = group.Find("shuffle"))
            shuffle = Parser::ParseBool(*pN);

        if(taskGridSize <= 0)
            throw std::runtime_error("Invalid taskGridSize value");
        
        return arena.Create<PathTracingRenderer>(workerCount, taskGridSize, epoch, shuffle, *integrator);
    }
    AGZ_HIERARCHY_WRAP("In creating path tracing renderer: " + group.ToString())
}

} // namespace Atrc::Mgr
