#pragma once

#include <Atrc/Camera/PerspectiveCamera.h>

#include <Atrc/Entity/GeometricEntity.h>
#include <Atrc/Entity/GeometricLightEntity.h>

#include <Atrc/Geometry/Sphere.h>
#include <Atrc/Geometry/TriangleBVH.h>

#include <Atrc/Material/BlackMaterial.h>
#include <Atrc/Material/DiffuseMaterial.h>
#include <Atrc/Material/FresnelSpecular.h>
#include <Atrc/Material/IdealMirror.h>

#include <Atrc/Light/DirectionalLight.h>
#include <Atrc/Light/GeometricDiffuseLightImpl.h>
#include <Atrc/Light/SkyLight.h>

#include <Atrc/Integrator/PathTracer.h>
#include <Atrc/Integrator/PureColorIntegrator.h>

#include <Atrc/Renderer/JitteredSubareaRenderer.h>

#include <Atrc/Renderer/ParallelRenderer.h>
#include <Atrc/Renderer/SerialRenderer.h>

AGZ_NS_BEG(Atrc)

using GeometricDiffuseLight = GeometricLightEntity<GeometricDiffuseLightImpl>;

AGZ_NS_END(Atrc)
