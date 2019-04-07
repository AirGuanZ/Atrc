#include <Atrc/Editor/Fresnel/Fresnel.h>

#include <Atrc/Editor/Fresnel/Dielectric.h>
#include <Atrc/Editor/Fresnel/Schlick.h>

void RegisterBuiltinFresnelCreators(FresnelFactory &factory)
{
    static const DielectricCreator iDielectricCreator;
    static const SchlickCreator iSchlickCreator;
    factory.AddCreator(&iDielectricCreator);
    factory.AddCreator(&iSchlickCreator);
}
