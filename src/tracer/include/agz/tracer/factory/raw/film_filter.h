#pragma once

#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

std::shared_ptr<FilmFilter> create_box_filter(
    real radius);
    
std::shared_ptr<FilmFilter> create_gaussian_filter(
    real radius, real alpha);

AGZ_TRACER_END
