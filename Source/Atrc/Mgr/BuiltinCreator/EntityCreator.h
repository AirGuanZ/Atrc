#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinEntityCreators(Context &context);

/*
    type = GeometricDiffuse

    geometry = GeometryDefinition
    radiance = Spectrum
*/
class GeometricDiffuseLightCreator : public Creator<Entity>
{
public:

    std::string GetTypeName() const override { return "GeometricDiffuse"; }

    Entity *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type     = Geometric
    
    geometry = GeometryDefinition
    material = MaterialDefinition
*/
class GeometricEntityCreator : public Creator<Entity>
{
public:

    std::string GetTypeName() const override { return "Geometric"; }

    Entity *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type               = GeometryGroup
    
    geometryGroup      = Name2GeometryDefinition
    materialAssignment = { MaterialAssignment }
    mediumAssignment   = { MediumAssignment }
    transform          = Transform
*/
class GeometryGroupEntityCreator : public Creator<Entity>
{
public:

    std::string GetTypeName() const override { return "GeometryGroup"; }

    Entity *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
