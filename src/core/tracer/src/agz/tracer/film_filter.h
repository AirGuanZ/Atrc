#pragma once

#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

FilmFilter *create_box_filter(
    real radius,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);
    
FilmFilter *create_gaussian_filter(
    real radius, real alpha,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
