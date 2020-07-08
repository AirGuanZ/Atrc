#pragma once

#include <agz/utility/texture.h>

#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/texture3d.h>

AGZ_TRACER_BEGIN

RC<Medium> create_heterogeneous_medium(
    const FTransform3 &local_to_world,
    RC<const Texture3D> density,
    RC<const Texture3D> albedo,
    RC<const Texture3D> g,
    int max_scattering_count);

RC<Medium> create_homogeneous_medium(
    const Spectrum &sigma_a,
    const Spectrum &sigma_s,
    real g,
    int max_scattering_count);

RC<Medium> create_void();

AGZ_TRACER_END
