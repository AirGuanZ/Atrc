#include <Atrc/Lib/Renderer/PathTracingIntegrator/NativePathTracingIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/ShadingNormalIntegrator.h>
#include <Atrc/Mgr/BuiltinCreator/PathTracingIntegratorCreator.h>

namespace Atrc::Mgr
{

void RegisterBuiltinPathTracingIntegratorCreators(Context &context)
{
    static const NativePathTracingIntegratorCreator nativePathTracingIntegratorCreator;
    static const ShadingNormalIntegratorCreator shadingNormalIntegratorCreator;
    context.AddCreator(&nativePathTracingIntegratorCreator);
    context.AddCreator(&shadingNormalIntegratorCreator);
}

PathTracingIntegrator *NativePathTracingIntegratorCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        int minDepth  = group["minDepth"].Parse<int>();
        int maxDepth  = group["maxDepth"].Parse<int>();
        Real contProb = group["contProb"].Parse<Real>();

        if(minDepth <= 0 || maxDepth < minDepth)
            throw MgrErr("Invalid min/max depth value");

        if(contProb <= 0 || contProb > 1)
            throw MgrErr("Invalid contProb value");

        return arena.Create<NativePathTracingIntegrator>(minDepth, maxDepth, contProb);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating native path tracing integrator: " + group.ToString())
}

PathTracingIntegrator *ShadingNormalIntegratorCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        return arena.Create<ShadingNormalIntegrator>();
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating shading normal integrator: " + group.ToString())
}

} // namespace Atrc::Mgr
