#include <Atrc/Lib/PostProcessor/ACESFilm.h>
#include <Atrc/Lib/PostProcessor/FlipImage.h>
#include <Atrc/Lib/PostProcessor/GammaCorrection.h>
#include <Atrc/Mgr/BuiltinCreator/PostProcessorCreator.h>

namespace Atrc::Mgr
{
    
void RegisterBuiltinPostProcessorCreators(Context& context)
{
    static ACESFilmCreator iAcesFilmCreator;
    static FlipImageCreator flipImageCreator;
    static GammaCorrectionCreator gammaCorrectionCreator;
    context.AddCreator(&iAcesFilmCreator);
    context.AddCreator(&flipImageCreator);
    context.AddCreator(&gammaCorrectionCreator);
}

PostProcessor *ACESFilmCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        return arena.Create<ACESFilm>();
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating post processor (aces film)")
}

PostProcessor *FlipImageCreator::Create([[maybe_unused]] const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        return arena.Create<FlipImage>();
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating post processor (flip image)")
}

PostProcessor *GammaCorrectionCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        Real gamma = group["gamma"].Parse<Real>();
        return arena.Create<GammaCorrection>(gamma);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating post processor (gamma correction)")
}

} // namespace Atrc::Mgr
