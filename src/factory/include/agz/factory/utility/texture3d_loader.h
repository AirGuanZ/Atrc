#pragma once

#include <fstream>

#include <agz/factory/context.h>

AGZ_TRACER_BEGIN

namespace texture3d_load
{

    /**
     * @brief load vol data from ascii file
     *
     * format：
     *
     * width : int
     * height: int
     * depth : int
     * for z in 0 to depth
     *     for y in 0 to height
     *         for x in 0 to width
     *             texel: float
     */
    texture::texture3d_t<real> load_gray_from_ascii(std::ifstream &fin);

    /**
     * @brief load vol data from binary file
     *
     * format:
     *
     * width : int32_t
     * height: int32_t
     * depth : int32_t
     * for z in 0 to depth
     *     for y in 0 to height
     *         for x in 0 to width
     *             texel: float
     */
    texture::texture3d_t<real> load_gray_from_binary(std::ifstream &fin);

    texture::texture3d_t<real> load_gray_from_images(
        const std::string *filenames, int image_count,
        const factory::PathMapper &path_mapper);

    /**
     * @brief load vol data from ascii file
     *
     * format:
     *
     * width : int
     * height: int
     * depth : int
     * for z in 0 to depth
     *     for y in 0 to height
     *         for x in 0 to width
     *             texel: float * SPECTRUM_COMPONENT_COUNT
     */
    texture::texture3d_t<Spectrum> load_rgb_from_ascii(std::ifstream &fin);

    /**
     * @brief load vol data from binary file
     *
     * format:
     *
     * width : int32_t
     * height: int32_t
     * depth : int32_t
     * for z in 0 to depth
     *     for y in 0 to height
     *         for x in 0 to width
     *             texel: float * SPECTRUM_COMPONENT_COUNT
     */
    texture::texture3d_t<Spectrum> load_rgb_from_binary(std::ifstream &fin);

    texture::texture3d_t<Spectrum> load_rgb_from_images(
        const std::string *filenames, int image_count,
        const factory::PathMapper &path_mapper);

    void save_gray_to_binary(
        const std::string &filename, const Vec3i &size, const float *data);

    void save_rgb_to_binary(
        const std::string &filename, const Vec3i &size, const float *data);

} // namespace texture3d_load

AGZ_TRACER_END
