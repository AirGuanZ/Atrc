#include <Atrc/Lib/Medium/HomogeneousMedium.h>
#include <Atrc/Mgr/BuiltinCreator/MediumCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace Atrc::Mgr
{

void RegisterBuiltinMediumCreators(Context &context)
{
    static HomogeneousMediumCreator homogeneousMediumCreator;
    context.AddCreator(&homogeneousMediumCreator);
}

Medium *HomogeneousMediumCreator::Create(const ConfigGroup &group, [[maybe_unused]] Context &context, Arena &arena) const
{
    ATRC_MGR_TRY
    {
        Spectrum sigmaA = Parser::ParseSpectrum(group["sigmaA"]);
        Spectrum sigmaS = Parser::ParseSpectrum(group["sigmaS"]);
        Spectrum le     = Parser::ParseSpectrum(group["le"]);
        Real g          = group["g"].Parse<Real>();
        return arena.Create<HomogeneousMedium>(sigmaA, sigmaS, le, g);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating homogeneous medium: " + group.ToString())
}

} // namespace Atrc::Mgr
