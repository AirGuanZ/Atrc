#pragma once

#include <agz/utility/texture.h>

#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/texture3d.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Medium> create_absorbtion_medium(
    const Spectrum &sigma_a);

std::shared_ptr<Medium> create_heterogeneous_medium(
    const Transform3 &local_to_world,
    std::shared_ptr<const Texture3D> density,
    std::shared_ptr<const Texture3D> albedo,
    std::shared_ptr<const Texture3D> g);

std::shared_ptr<Medium> create_homogeneous_medium(
    const Spectrum &sigma_a,
    const Spectrum &sigma_s,
    real g);

std::shared_ptr<Medium> create_void();

AGZ_TRACER_END
