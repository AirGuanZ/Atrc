#pragma once

#include <agz/common/math.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

/**
 * @brief gbuffer的单个像素
 */
struct GBufferPixel
{
    Spectrum albedo;
    Vec3 position;
    Vec3 normal;
    real depth  = 0;
    real binary = 0;
};

/**
 * @brief 各类gbuffer分量
 */
using AlbedoBuffer   = texture::texture2d_t<Spectrum>;
using PositionBuffer = texture::texture2d_t<Vec3>;
using NormalBuffer   = texture::texture2d_t<Vec3>;
using DepthBuffer    = texture::texture2d_t<real>;
using BinaryBuffer   = texture::texture2d_t<real>;

/**
 * @brief 完整的gbuffer
 */
struct GBuffer
{
    std::unique_ptr<AlbedoBuffer>   albedo;
    std::unique_ptr<PositionBuffer> position;
    std::unique_ptr<NormalBuffer>   normal;
    std::unique_ptr<DepthBuffer>    depth;
    std::unique_ptr<BinaryBuffer>   binary;

    GBuffer()                              = default;
    GBuffer(GBuffer&&)            noexcept = default;
    GBuffer &operator=(GBuffer&&) noexcept = default;

    GBuffer(const GBuffer &copy_from)
    {
        albedo   = std::make_unique<AlbedoBuffer>  (*copy_from.albedo);
        position = std::make_unique<PositionBuffer>(*copy_from.position);
        normal   = std::make_unique<NormalBuffer>  (*copy_from.normal);
        depth    = std::make_unique<DepthBuffer>   (*copy_from.depth);
        binary   = std::make_unique<BinaryBuffer>  (*copy_from.binary);
    }

    GBuffer &operator=(const GBuffer &copy_from)
    {
        albedo   = std::make_unique<AlbedoBuffer>  (*copy_from.albedo);
        position = std::make_unique<PositionBuffer>(*copy_from.position);
        normal   = std::make_unique<NormalBuffer>  (*copy_from.normal);
        depth    = std::make_unique<DepthBuffer>   (*copy_from.depth);
        binary   = std::make_unique<BinaryBuffer>  (*copy_from.binary);
        return *this;
    }

    GBuffer(int h, int w)
    {
        albedo   = std::make_unique<AlbedoBuffer>  (h, w);
        position = std::make_unique<PositionBuffer>(h, w);
        normal   = std::make_unique<NormalBuffer>  (h, w);
        depth    = std::make_unique<DepthBuffer>   (h, w);
        binary   = std::make_unique<BinaryBuffer>  (h, w);
    }

    GBufferPixel get(int y, int x) const
    {
        GBufferPixel ret;
        if(albedo)   ret.albedo   = albedo  ->at(y, x);
        if(position) ret.position = position->at(y, x);
        if(normal)   ret.normal   = normal  ->at(y, x);
        if(depth)    ret.depth    = depth   ->at(y, x);
        if(binary)   ret.binary   = binary  ->at(y, x);
        return ret;
    }

    void set(int y, int x, const GBufferPixel &pixel)
    {
        if(albedo)   albedo  ->at(y, x) = pixel.albedo;
        if(position) position->at(y, x) = pixel.position;
        if(normal)   normal  ->at(y, x) = pixel.normal;
        if(depth)    depth   ->at(y, x) = pixel.depth;
        if(binary)   binary  ->at(y, x) = pixel.binary;
    }
};

AGZ_TRACER_END
