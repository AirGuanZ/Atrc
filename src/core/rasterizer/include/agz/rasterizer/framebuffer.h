#pragma once

#include <agz/rasterizer/common.h>

AGZ_RASTERIZER_BEGIN

using ColorBuffer = ImageBuffer<RGBA>;
using DepthBuffer = ImageBuffer<real>;

template<typename...PixelFormats>
class Framebuffer
{
    using Tuple = std::tuple<ImageBuffer<PixelFormats>*...>;

    Tuple color_buffers_;
    DepthBuffer *depth_buffer_;

    int width_, height_;

public:

    explicit Framebuffer(ImageBuffer<PixelFormats>*...color_buffers, DepthBuffer *depth_buffer) noexcept
        : color_buffers_{ color_buffers... }, depth_buffer_(depth_buffer)
    {
        if(depth_buffer)
        {
            width_  = depth_buffer_->width();
            height_ = depth_buffer_->height();
        }
        else
        {
            auto &color = color_buffer<0>();
            width_  = color.width();
            height_ = color.height();
        }
    }

    template<int I>
    std::remove_pointer_t<std::tuple_element_t<I, Tuple>> &color_buffer() noexcept
    {
        return *std::get<I>(color_buffers_);
    }

    template<int I>
    const std::remove_pointer_t<std::tuple_element_t<I, Tuple>> &color_buffer() const noexcept
    {
        return *std::get<I>(color_buffers_);
    }

    DepthBuffer &depth_buffer() noexcept
    {
        return *depth_buffer_;
    }

    const DepthBuffer &depth_buffer() const noexcept
    {
        return *depth_buffer_;
    }

    bool has_depth_buffer() const noexcept
    {
        return depth_buffer_ != nullptr;
    }

    int color_buffer_count() const noexcept
    {
        return static_cast<int>(std::tuple_size_v<Tuple>);
    }

    int width() const noexcept
    {
        return width_;
    }

    int height() const noexcept
    {
        return height_;
    }
};

AGZ_RASTERIZER_END
