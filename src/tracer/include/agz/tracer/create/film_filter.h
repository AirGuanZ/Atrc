#pragma once

#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

RC<FilmFilter> create_box_filter(
    real radius);
    
RC<FilmFilter> create_gaussian_filter(
    real radius, real alpha);

AGZ_TRACER_END
