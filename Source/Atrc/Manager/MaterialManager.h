#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class MaterialCreator
{
public:

    virtual ~MaterialCreator() = default;

    virtual Material *Create(const SceneParamGroup &params, AGZ::ObjArena<> &arena) const = 0;
};

class MaterialManager
{
    std::unordered_map<Str8, const MaterialCreator*> name2Mat_;

public:

    void SetParser(const Str8 &name, const MaterialCreator *parser);

    Material *Create(const Str8 &name, const SceneParamGroup &params, AGZ::ObjArena<> &arena);
};

#define DEFINE_MATERIAL_CREATOR(NAME) \
    class NAME : public MaterialCreator \
    { \
    public: \
        Material *Create(const SceneParamGroup &params, AGZ::ObjArena<> &arena) const override; \
    }

DEFINE_MATERIAL_CREATOR(BlackMaterialCreator     );
DEFINE_MATERIAL_CREATOR(DiffuseMaterialCreator   );
DEFINE_MATERIAL_CREATOR(FresnelSpecularCreator   );
DEFINE_MATERIAL_CREATOR(IdealMirrorCreator       );
DEFINE_MATERIAL_CREATOR(MetalCreator             );
DEFINE_MATERIAL_CREATOR(PlasticCreator           );
DEFINE_MATERIAL_CREATOR(TextureScalerCreator     );
DEFINE_MATERIAL_CREATOR(UncallableMaterialCreator);

#undef DEFINE_MATERIAL_CREATOR

AGZ_NS_END(Atrc)
