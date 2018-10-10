#pragma once

#include <Atrc/Camera/ApertureCamera.h>
#include <Atrc/Camera/PerspectiveCamera.h>

#include <Atrc/Math/Math.h>

#include <Atrc/Integrator/AmbientIntegrator.h>
#include <Atrc/Integrator/PathTracer.h>

#include <Atrc/Entity/GeometryTemplate/Cube.h>
#include <Atrc/Entity/GeometryTemplate/Sphere.h>
#include <Atrc/Entity/GeometryTemplate/SmoothTriangle.h>
#include <Atrc/Entity/GeometryTemplate/Transformer.h>
#include <Atrc/Entity/GeometryTemplate/TriangleBVH.h>

#include <Atrc/Entity/BruteforceTriangleMesh.h>
#include <Atrc/Entity/ColoredSky.h>
#include <Atrc/Entity/MatGeoEntity.h>

#include <Atrc/Material/Ambient.h>
#include <Atrc/Material/Diffuse.h>
#include <Atrc/Material/Glass.h>
#include <Atrc/Material/MatOpr.h>
#include <Atrc/Material/Metal.h>
#include <Atrc/Material/Normal.h>

#include <Atrc/Renderer/ParallelRenderer.h>
#include <Atrc/Renderer/SerialRenderer.h>

#include <Atrc/Renderer/JitteredSubareaRenderer.h>
#include <Atrc/Renderer/Native1sppSubareaRenderer.h>
