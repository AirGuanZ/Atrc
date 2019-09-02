#include "./directional_light.h"
#include "./envir_light.h"
#include "./native_sky.h"

AGZ_TRACER_BEGIN

using DirectionalLightEntity = InfiniteLightEntity<InfiniteLightImplToLight<DirectionalLightImpl>>;
using EnvironmentLightEntity = InfiniteLightEntity<InfiniteLightImplToLight<EnvironmentLightImpl>>;
using NativeSkyEntity        = InfiniteLightEntity<InfiniteLightImplToLight<NativeSkyLightImpl>>;

using InfiniteLightAggregateEntity = InfiniteLightEntity<InfiniteLightImplAggregate>;
AGZT_IMPLEMENTATION(Entity, InfiniteLightAggregateEntity, "aggregate")

AGZT_IMPLEMENTATION(InfiniteLightImpl, DirectionalLightImpl,  "dir");
AGZT_IMPLEMENTATION(InfiniteLightImpl, EnvironmentLightImpl , "env");
AGZT_IMPLEMENTATION(InfiniteLightImpl, NativeSkyLightImpl,    "native_sky");

AGZT_IMPLEMENTATION(Entity, DirectionalLightEntity, "dir")
AGZT_IMPLEMENTATION(Entity, EnvironmentLightEntity, "env");
AGZT_IMPLEMENTATION(Entity, NativeSkyEntity,        "native_sky");

AGZ_TRACER_END
