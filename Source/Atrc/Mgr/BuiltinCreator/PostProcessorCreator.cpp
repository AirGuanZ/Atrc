#include <Atrc/Core/PostProcessor/ACESFilm.h>
#include <Atrc/Core/PostProcessor/FlipImage.h>
#include <Atrc/Core/PostProcessor/GammaCorrection.h>
#include <Atrc/Core/PostProcessor/NativeToneMapping.h>
#include <Atrc/Core/PostProcessor/SaveAsHDR.h>
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
    static const SaveAsHDRCreator saveAsHdr;
    context.AddCreator(&iAcesFilmCreator);
    context.AddCreator(&flipImageCreator);
    context.AddCreator(&gammaCorrectionCreator);
    context.AddCreator(&nativeToneMappingCreator);
    context.AddCreator(&saveAsHdr);
}

PostProcessor *ACESFilmCreator::Create([[maybe_unused]] const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        return arena.Create<ACESFilm>();
    }
    AGZ_HIERARCHY_WRAP("In creating post processor (aces film)")
}

PostProcessor *FlipImageCreator::Create([[maybe_unused]] const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        return arena.Create<FlipImage>();
    }
    AGZ_HIERARCHY_WRAP("In creating post processor (flip image)")
}

PostProcessor *GammaCorrectionCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        Real gamma = group["gamma"].Parse<Real>();
        return arena.Create<GammaCorrection>(gamma);
    }
    AGZ_HIERARCHY_WRAP("In creating post processor (gamma correction)")
}

PostProcessor* NativeToneMappingCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        auto LWhite = Parser::ParseSpectrum(group["LWhite"]);
        return arena.Create<NativeToneMapping>(LWhite);
    }
    AGZ_HIERARCHY_WRAP("In creating post processor (native tone mapping)")
}

PostProcessor *SaveAsHDRCreator::Create(const ConfigGroup &group, Context &context, Arena &arena) const
{
    AGZ_HIERARCHY_TRY
    {
        std::string filename = context.GetPathInWorkspace(group["filename"].AsValue());
        return arena.Create<SaveAsHDR>(filename);
    }
    AGZ_HIERARCHY_WRAP("In creating post processor (save as hdr)")
}

} // namespace Atrc::Mgr
