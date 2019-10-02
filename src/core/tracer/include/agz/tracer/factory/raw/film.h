#pragma once

#include <agz/tracer/core/film.h>
#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

std::shared_ptr<Film> create_filtered_film(
    int width, int height,
    std::shared_ptr<const FilmFilter> film_filter);

std::shared_ptr<Film> create_native_film(
    int width, int height);

AGZ_TRACER_END
