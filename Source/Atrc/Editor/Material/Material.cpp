#include <Atrc/Editor/Material/Material.h>

#include <Atrc/Editor/Material/IdealDiffuse.h>

void RegisterBuiltinMaterialCreators(MaterialFactory &factory)
{
    static const IdealDiffuseCreator iIdealDiffuseCreator;
    factory.AddCreator(&iIdealDiffuseCreator);
}
