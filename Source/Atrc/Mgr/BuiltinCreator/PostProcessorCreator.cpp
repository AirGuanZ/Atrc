#include <Atrc/Lib/PostProcessor/ACESFilm.h>
#include <Atrc/Lib/PostProcessor/FlipImage.h>
#include <Atrc/Lib/PostProcessor/GammaCorrection.h>
#include <Atrc/Lib/PostProcessor/NativeToneMapping.h>
#include <Atrc/Mgr/Parser.h>
#include <Atrc/Mgr/BuiltinCreator/PostProcessorCreator.h>

namespace Atrc::Mgr
{
    
void RegisterBuiltinPostProcessorCreators(Context& context)
{
    static const ACESFilmCreator iAcesFilmCreator;
    static const FlipImageCreator flipImageCreator;
    static const GammaCorrectionCreator gammaCorrectionCreator;
    static const NativeToneMappingCreator nativeToneMappingCreator;
    context.AddCreator(&iAcesFilmCreator);
    context.AddCreator(&flipImageCreator);
    context.AddCreator(&gammaCorrectionCreator);
    context.AddCreator(&nativeToneMappingCreator);
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

PostProcessor* NativeToneMappingCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        auto LWhite = Parser::ParseSpectrum(group["LWhite"]);
        return arena.Create<NativeToneMapping>(LWhite);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating post processor (native tone mapping)")
}

} // namespace Atrc::Mgr
