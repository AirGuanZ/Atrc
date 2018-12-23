#include <Atrc/Lib/Renderer/PathTracingIntegrator/MISPathTracingIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/NativePathTracingIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/ShadingNormalIntegrator.h>
#include <Atrc/Lib/Renderer/PathTracingIntegrator/VolumetricPathTracingIntegrator.h>
#include <Atrc/Mgr/BuiltinCreator/PathTracingIntegratorCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinPathTracingIntegratorCreators(Context &context)
{
    static const MISPathTracingIntegratorCreator iMISPathTracingIntegratorCreator;
    static const NativePathTracingIntegratorCreator nativePathTracingIntegratorCreator;
    static const ShadingNormalIntegratorCreator shadingNormalIntegratorCreator;
    static const VolumetricPathTracingIntegratorCreator volumetricPathTracingIntegratorCreator;
    static const VolPathTracingIntegratorCreator volPathTracingIntegratorCreator;
    context.AddCreator(&iMISPathTracingIntegratorCreator);
    context.AddCreator(&nativePathTracingIntegratorCreator);
    context.AddCreator(&shadingNormalIntegratorCreator);
    context.AddCreator(&volumetricPathTracingIntegratorCreator);
    context.AddCreator(&volPathTracingIntegratorCreator);
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

PathTracingIntegrator *VolumetricPathTracingIntegratorCreator::Create(
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

        return arena.Create<VolumetricPathTracingIntegrator>(
            minDepth, maxDepth, contProb, sampleAllLights);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating volumetric path tracing integrator: " + group.ToString())
}

PathTracingIntegrator *VolPathTracingIntegratorCreator::Create(
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

        return arena.Create<VolumetricPathTracingIntegrator>(
            minDepth, maxDepth, contProb, sampleAllLights);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating volumetric path tracing integrator: " + group.ToString())
}

} // namespace Atrc::Mgr
