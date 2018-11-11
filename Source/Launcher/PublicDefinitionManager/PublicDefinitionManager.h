#pragma once

#include "../Common.h"

#include "../EntityManager/EntityCreator.h"
#include "../GeometryManager/GeometryManager.h"
#include "../LightManager/LightManager.h"
#include "../IntegratorManager/IntegratorManager.h"
#include "../MaterialManager/MaterialManager.h"
#include "../MediumManager/MediumManager.h"
#include "../RendererManager/RendererManager.h"

using EntityDefinitionManager          = PublicDefinitionManager<Atrc::Entity>;
using GeometryDefinitionManager        = PublicDefinitionManager<Atrc::Geometry>;
using IntegratorDefinitionManager      = PublicDefinitionManager<Atrc::Integrator>;
using LightDefinitionManager           = PublicDefinitionManager<Atrc::Light>;
using MaterialDefinitionManager        = PublicDefinitionManager<Atrc::Material>;
using MediumDefinitionManager          = PublicDefinitionManager<Atrc::Medium>;
using RendererDefinitionManager        = PublicDefinitionManager<Atrc::Renderer>;
using SubareaRendererDefinitionManager = PublicDefinitionManager<Atrc::SubareaRenderer>;
