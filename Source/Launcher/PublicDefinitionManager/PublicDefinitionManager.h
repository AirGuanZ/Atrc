#pragma once

#include "../Common.h"

#include "../EntityManager/EntityCreator.h"
#include "../GeometryManager/GeometryManager.h"
#include "../LightManager/LightManager.h"
#include "../IntegratorManager/IntegratorManager.h"
#include "../MaterialManager/MaterialManager.h"
#include "../MediumManager/MediumManager.h"
#include "../RendererManager/RendererManager.h"

template<typename T>
class PublicDefinitionManager : public AGZ::Singleton<PublicDefinitionManager<T>>
{
    std::unordered_map<Str8, const T*> definitions_;

public:

    void Initialize(const ConfigGroup &contents, ObjArena<> &arena)
    {
        for(auto it : contents.GetChildren())
            definitions_[it.first] = ObjectManager<T>::GetInstance().Create(it.first);
    }

    const T *Get(const Str8 &name) const
    {
        auto it = definitions_.find(name);
        return it != definitions_.end() ? it->second : nullptr;
    }
};

using EntityDefinitionManager          = PublicDefinitionManager<Atrc::Entity>;
using GeometryDefinitionManager        = PublicDefinitionManager<Atrc::Geometry>;
using IntegratorDefinitionManager      = PublicDefinitionManager<Atrc::Integrator>;
using LightDefinitionManager           = PublicDefinitionManager<Atrc::Light>;
using MaterialDefinitionManager        = PublicDefinitionManager<Atrc::Material>;
using MediumDefinitionManager          = PublicDefinitionManager<Atrc::Medium>;
using RendererDefinitionManager        = PublicDefinitionManager<Atrc::Renderer>;
using SubareaRendererDefinitionManager = PublicDefinitionManager<Atrc::SubareaRenderer>;