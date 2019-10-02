#pragma once

#include <agz/tracer/core/medium.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Medium> create_absorbtion_medium(
    const Spectrum &sigma_a);

std::shared_ptr<Medium> create_homogeneous_medium(
    const Spectrum &sigma_a,
    const Spectrum &sigma_s,
    real g);

std::shared_ptr<Medium> create_void();

AGZ_TRACER_END
