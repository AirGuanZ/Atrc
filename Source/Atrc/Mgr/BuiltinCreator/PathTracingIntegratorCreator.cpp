#include <Atrc/Lib/Renderer/PathTracingIntegrator/FullPathTracingIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/MISPathTracingIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/NativePathTracingIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/ShadingNormalIntegrator.h>
#include <Atrc/Mgr/BuiltinCreator/PathTracingIntegratorCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinPathTracingIntegratorCreators(Context &context)
{
    static const FullPathTracingIntegratorCreator fullPathTracingIntegrator;
    static const MISPathTracingIntegratorCreator iMISPathTracingIntegratorCreator;
    static const NativePathTracingIntegratorCreator nativePathTracingIntegratorCreator;
    static const ShadingNormalIntegratorCreator shadingNormalIntegratorCreator;
    context.AddCreator(&fullPathTracingIntegrator);
    context.AddCreator(&iMISPathTracingIntegratorCreator);
    context.AddCreator(&nativePathTracingIntegratorCreator);
    context.AddCreator(&shadingNormalIntegratorCreator);
}

PathTracingIntegrator *FullPathTracingIntegratorCreator::Create(
    const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        int minDepth = group["minDepth"].Parse<int>();
        int maxDepth = group["maxDepth"].Parse<int>();
        Real contProb = group["contProb"].Parse<Real>();
        bool sampleAllLights = Parser::ParseBool(group["sampleAllLights"]);

        if(minDepth <= 0 || maxDepth < minDepth)
            throw MgrErr("Invalid min/max depth value");

        if(contProb <= 0 || contProb > 1)
            throw MgrErr("Invalid contProb value");

        return arena.Create<FullPathTracingIntegrator>(
            minDepth, maxDepth, contProb, sampleAllLights);
    }
        ATRC_MGR_CATCH_AND_RETHROW("In creating full path tracing integrator: " + group.ToString())
}

PathTracingIntegrator *MISPathTracingIntegratorCreator::Create(
    const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        int minDepth  = group["minDepth"].Parse<int>();
        int maxDepth  = group["maxDepth"].Parse<int>();
        Real contProb = group["contProb"].Parse<Real>();
        bool sampleAllLights = Parser::ParseBool(group["sampleAllLights"]);

        if(minDepth <= 0 || maxDepth < minDepth)
            throw MgrErr("Invalid min/max depth value");

        if(contProb <= 0 || contProb > 1)
            throw MgrErr("Invalid contProb value");

        return arena.Create<MISPathTracingIntegrator>(
            minDepth, maxDepth, contProb, sampleAllLights);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating MIS path tracing integrator: " + group.ToString())
}

PathTracingIntegrator *NativePathTracingIntegratorCreator::Create(
    const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
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

PathTracingIntegrator *ShadingNormalIntegratorCreator::Create(
    const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        return arena.Create<ShadingNormalIntegrator>();
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating shading normal integrator: " + group.ToString())
}

} // namespace Atrc::Mgr
