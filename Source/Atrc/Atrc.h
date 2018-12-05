#pragma once

#include <Atrc/Accelarator/BVH.h>

#include <Atrc/Camera/PinholeCamera.h>
#include <Atrc/Camera/ThinLensCamera.h>

#include <Atrc/Entity/GeometricEntity.h>
#include <Atrc/Entity/GeometricLightEntity.h>

#include <Atrc/Geometry/Cube.h>
#include <Atrc/Geometry/Sphere.h>
#include <Atrc/Geometry/TriangleBVH.h>

#include <Atrc/Light/CubeEnvironmentLight.h>
#include <Atrc/Light/DirectionalLight.h>
#include <Atrc/Light/GeometricDiffuseLightImpl.h>
#include <Atrc/Light/SkyLight.h>
#include <Atrc/Light/SphereEnvironmentLight.h>

#include <Atrc/Material/BlackMaterial.h>
#include <Atrc/Material/DiffuseMaterial.h>
#include <Atrc/Material/FresnelSpecular.h>
#include <Atrc/Material/IdealMirror.h>
#include <Atrc/Material/Metal.h>
#include <Atrc/Material/Plastic.h>
#include <Atrc/Material/TextureScaler.h>
#include <Atrc/Material/UncallableMaterial.h>

#include <Atrc/Medium/HomogeneousMedium.h>

#include <Atrc/PostProcessor/ACESFilm.h>
#include <Atrc/PostProcessor/GammaCorrector.h>
#include <Atrc/PostProcessor/HorizontalFlipper.h>
#include <Atrc/PostProcessor/VerticalFlipper.h>

#include <Atrc/Renderer/PathTracingIntegrator/AmbientOcclusionIntegrator.h>
#include <Atrc/Renderer/PathTracingIntegrator/PathTracer.h>
#include <Atrc/Renderer/PathTracingIntegrator/PureColorIntegrator.h>
#include <Atrc/Renderer/PathTracingIntegrator/VolumetricPathTracer.h>

#include <Atrc/Renderer/DefaultProgressReporter.h>
#include <Atrc/Renderer/PathTracingRenderer.h>

AGZ_NS_BEG(Atrc)

using GeometricDiffuseLight = GeometricLightEntity<GeometricDiffuseLightImpl>;

AGZ_NS_END(Atrc)
