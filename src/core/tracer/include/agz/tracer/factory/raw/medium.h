#pragma once

#include <agz/utility/texture.h>

#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/texture3d.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Medium> create_absorbtion_medium(
    const Spectrum &sigma_a);

std::shared_ptr<Medium> create_homogeneous_medium(
    const Spectrum &sigma_a,
    const Spectrum &sigma_s,
    real g);

std::shared_ptr<Medium> create_void();

std::shared_ptr<Medium> create_grid_medium(
    const Transform3 &local_to_world,
    std::shared_ptr<const Texture3D> density,
    real sigma_a, real sigma_s, real g);

AGZ_TRACER_END
