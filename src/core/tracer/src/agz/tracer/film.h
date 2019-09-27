#pragma once

#include <agz/tracer/core/film.h>
#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

Film *create_filtered_film(
    int width, int height,
    const FilmFilter *film_filter,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

Film *create_native_film(
    int width, int height,
    obj::Object::CustomedFlag customed_flag,
    Arena &arena);

AGZ_TRACER_END
