#include <Atrc/Lib/Renderer/PathTracingRenderer.h>
#include <Atrc/Mgr/BuiltinCreator/RendererCreator.h>

namespace Atrc::Mgr
{

const Renderer *PathTracingRendererCreator::Create(const ConfigGroup &group, Context &context, Arena &arena)
{
    ATRC_MGR_TRY
    {
        auto integrator  = context.Create<PathTracingIntegrator>(group["integrator"]);
        int workerCount  = group["workerCount"].Parse<int>();
        int taskGridSize = group["taskGridSize"].Parse<int>();

        if(taskGridSize <= 0)
            throw MgrErr("Invalid taskGridSize value");
        
        return arena.Create<PathTracingRenderer>(workerCount, taskGridSize, *integrator);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating path tracing renderer: " + group.ToString())
}

} // namespace Atrc::Mgr
