#include <chrono>

#include <Atrc/Core/Sampler/NativeSampler.h>
#include <Atrc/Mgr/BuiltinCreator/SamplerCreator.h>

namespace Atrc::Mgr
{

void RegisterBuiltinSamplerCreators(Context &context)
{
    static const NativeSamplerCreator nativeSamplerCreator;
    context.AddCreator(&nativeSamplerCreator);
}

Sampler *NativeSamplerCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        int seed;
        if(auto seedNode = group.Find("seed"))
            seed = seedNode->Parse<int>();
        else
            seed = int(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        
        int spp = group["spp"].Parse<int>();
        if(spp <= 0)
            throw AGZ::HierarchyException("Invalid spp value");

        return arena.Create<NativeSampler>(seed, spp);
    }
    AGZ_HIERARCHY_WRAP("In creating native sampler: " + group.ToString())
}

} // namespace Atrc::Mgr
