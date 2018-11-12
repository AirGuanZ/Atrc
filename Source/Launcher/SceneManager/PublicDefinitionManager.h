#pragma once

#include "../Common.h"

#include "EntityCreator.h"
#include "GeometryManager.h"
#include "LightManager.h"
#include "IntegratorManager.h"
#include "MaterialManager.h"
#include "MediumManager.h"
#include "RendererManager.h"

using EntityDefinitionManager          = PublicDefinitionManager<Atrc::Entity>;
using GeometryDefinitionManager        = PublicDefinitionManager<Atrc::Geometry>;
using IntegratorDefinitionManager      = PublicDefinitionManager<Atrc::Integrator>;
using LightDefinitionManager           = PublicDefinitionManager<Atrc::Light>;
using MaterialDefinitionManager        = PublicDefinitionManager<Atrc::Material>;
using MediumDefinitionManager          = PublicDefinitionManager<Atrc::Medium>;
using RendererDefinitionManager        = PublicDefinitionManager<Atrc::Renderer>;
using SubareaRendererDefinitionManager = PublicDefinitionManager<Atrc::SubareaRenderer>;
