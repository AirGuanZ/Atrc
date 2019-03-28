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
    AGZ_HIERARCHY_TRY
    {
        Spectrum sigmaA = Parser::ParseSpectrum(group["sigmaA"]);
        Spectrum sigmaS = Parser::ParseSpectrum(group["sigmaS"]);
        Spectrum le     = Parser::ParseSpectrum(group["le"]);
        Real g          = group["g"].Parse<Real>();
        return arena.Create<HomogeneousMedium>(sigmaA, sigmaS, le, g);
    }
    AGZ_HIERARCHY_WRAP("In creating homogeneous medium: " + group.ToString())
}

} // namespace Atrc::Mgr
