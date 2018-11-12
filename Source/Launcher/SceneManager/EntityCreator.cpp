#include "../ParamParser/ParamParser.h"
#include "EntityCreator.h"

namespace
{
    Atrc::MediumInterface ParseMediumInterface(const ConfigGroup &params, ObjArena<> &arena)
    {
        auto inGrp  = params.Find("in");
        auto outGrp = params.Find("out");

        auto in  = inGrp  ? GetSceneObject<Atrc::Medium>(*inGrp, arena) : nullptr;
        auto out = outGrp ? GetSceneObject<Atrc::Medium>(*outGrp, arena) : nullptr;

        Atrc::MediumInterface ret;
        ret.in  = in;
        ret.out = out;
        return ret;
    }
}

Atrc::Entity *GeometricEntityCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto geometry = GetSceneObject<Atrc::Geometry>(params["geometry"], arena);
    auto material = GetSceneObject<Atrc::Material>(params["material"], arena);

    auto mediumGrp = params.Find("mediumInterface");
    auto medium = mediumGrp ? ParseMediumInterface(mediumGrp->AsGroup(), arena) : Atrc::MediumInterface{ nullptr, nullptr };

	return arena.Create<Atrc::GeometricEntity>(geometry, material, medium);
}

Atrc::Entity *GeometricDiffuseLightCreator::Create(const ConfigGroup &params, ObjArena<> &arena) const
{
    auto geometry = GetSceneObject<Atrc::Geometry>(params["geometry"], arena);
	auto radiance = ParamParser::ParseSpectrum(params["radiance"]);

    auto mediumGrp = params.Find("mediumInterface");
    auto medium = mediumGrp ? ParseMediumInterface(mediumGrp->AsGroup(), arena) : Atrc::MediumInterface{ nullptr, nullptr };

	return arena.Create<Atrc::GeometricDiffuseLight>(medium, geometry, radiance);
}
