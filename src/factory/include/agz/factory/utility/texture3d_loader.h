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
    texture::texture3d_t<real> load_real_from_ascii(std::ifstream &fin);

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
    texture::texture3d_t<real> load_real_from_binary(std::ifstream &fin);

    /**
     * @brief load vol data from images
     */
    texture::texture3d_t<real> load_real_from_images(
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
    texture::texture3d_t<Spectrum> load_spec_from_ascii(std::ifstream &fin);

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
    texture::texture3d_t<Spectrum> load_spec_from_binary(std::ifstream &fin);

    /**
     * @brief load vol data from images
     */
    texture::texture3d_t<Spectrum> load_spec_from_images(
        const std::string *filenames, int image_count,
        const factory::PathMapper &path_mapper);

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
     *             texel: uint8
     */
    texture::texture3d_t<uint8_t> load_uint8_from_ascii(std::ifstream &fin);

    /**
     * @brief load vol data from binary file
     *
     * format：
     *
     * width : int
     * height: int
     * depth : int
     * for z in 0 to depth
     *     for y in 0 to height
     *         for x in 0 to width
     *             texel: uint8
     */
    texture::texture3d_t<uint8_t> load_uint8_from_binary(std::ifstream &fin);

    /**
     * @brief load vol data from images
     */
    texture::texture3d_t<uint8_t> load_uint8_from_images(
        const std::string *filenames, int image_count,
        const factory::PathMapper &path_mapper);

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
     *             texel: uint8 * 3
     */
    texture::texture3d_t<math::color3b> load_uint24_from_ascii(std::ifstream &fin);

    /**
     * @brief load vol data from binary file
     *
     * format：
     *
     * width : int
     * height: int
     * depth : int
     * for z in 0 to depth
     *     for y in 0 to height
     *         for x in 0 to width
     *             texel: uint8 * 3
     */
    texture::texture3d_t<math::color3b> load_uint24_from_binary(std::ifstream &fin);

    /**
     * @brief load vol data from images
     */
    texture::texture3d_t<math::color3b> load_uint24_from_images(
        const std::string *filenames, int image_count,
        const factory::PathMapper &path_mapper);

    /**
     * @brief save vol data to binary file
     */
    void save_real_to_binary(
        const std::string &filename, const Vec3i &size, const float *data);

    /**
     * @brief save vol data to binary file
     */
    void save_spec_to_binary(
        const std::string &filename, const Vec3i &size, const float *data);

    /**
     * @brief save vol data to binary file
     */
    void save_uint8_to_binary(
        const std::string &filename, const Vec3i &size, const uint8_t *data);

    /**
     * @brief save vol data to binary file
     */
    void save_uint24_to_binary(
        const std::string &filename, const Vec3i &size, const math::color3b *data);

} // namespace texture3d_load

AGZ_TRACER_END
