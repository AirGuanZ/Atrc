#include <Atrc/Core/Material.h>

AGZ_NS_BEG(Atrc)

Material *Material::Clone(const SceneParamGroup &group, AGZ::ObjArena<> &arena) const
{
    throw UnimplementedMethod("Material::Clone: unimplemented");
}

AGZ_NS_END(Atrc)
