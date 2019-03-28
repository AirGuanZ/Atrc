#include <Atrc/Core/Renderer/PathTracingIntegrator/FullPathTracingIntegrator.h>
#include <Atrc/Core/Renderer/PathTracingIntegrator/MISPathTracingIntegrator.h>
#include <Atrc/Core/Renderer/PathTracingIntegrator/NativePathTracingIntegrator.h>
#include <Atrc/Core/Renderer/PathTracingIntegrator/ShadingNormalIntegrator.h>
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
    AGZ_HIERARCHY_TRY
    {
        int minDepth = group["minDepth"].Parse<int>();
        int maxDepth = group["maxDepth"].Parse<int>();
        Real contProb = group["contProb"].Parse<Real>();
        bool sampleAllLights = Parser::ParseBool(group["sampleAllLights"]);

        if(minDepth <= 0 || maxDepth < minDepth)
            throw AGZ::HierarchyException("Invalid min/max depth value");

        if(contProb <= 0 || contProb > 1)
            throw AGZ::HierarchyException("Invalid contProb value");

        return arena.Create<FullPathTracingIntegrator>(
            minDepth, maxDepth, contProb, sampleAllLights);
    }
        AGZ_HIERARCHY_WRAP("In creating full path tracing integrator: " + group.ToString())
}

PathTracingIntegrator *MISPathTracingIntegratorCreator::Create(
    const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        int minDepth  = group["minDepth"].Parse<int>();
        int maxDepth  = group["maxDepth"].Parse<int>();
        Real contProb = group["contProb"].Parse<Real>();
        bool sampleAllLights = Parser::ParseBool(group["sampleAllLights"]);

        if(minDepth <= 0 || maxDepth < minDepth)
            throw AGZ::HierarchyException("Invalid min/max depth value");

        if(contProb <= 0 || contProb > 1)
            throw AGZ::HierarchyException("Invalid contProb value");

        return arena.Create<MISPathTracingIntegrator>(
            minDepth, maxDepth, contProb, sampleAllLights);
    }
    AGZ_HIERARCHY_WRAP("In creating MIS path tracing integrator: " + group.ToString())
}

PathTracingIntegrator *NativePathTracingIntegratorCreator::Create(
    const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        int minDepth  = group["minDepth"].Parse<int>();
        int maxDepth  = group["maxDepth"].Parse<int>();
        Real contProb = group["contProb"].Parse<Real>();

        if(minDepth <= 0 || maxDepth < minDepth)
            throw AGZ::HierarchyException("Invalid min/max depth value");

        if(contProb <= 0 || contProb > 1)
            throw AGZ::HierarchyException("Invalid contProb value");

        return arena.Create<NativePathTracingIntegrator>(minDepth, maxDepth, contProb);
    }
    AGZ_HIERARCHY_WRAP("In creating native path tracing integrator: " + group.ToString())
}

PathTracingIntegrator *ShadingNormalIntegratorCreator::Create(
    const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        return arena.Create<ShadingNormalIntegrator>();
    }
    AGZ_HIERARCHY_WRAP("In creating shading normal integrator: " + group.ToString())
}

} // namespace Atrc::Mgr
