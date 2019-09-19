#pragma once

#include <agz/rasterizer/common.h>
#include <agz/rasterizer/varying.h>

AGZ_RASTERIZER_BEGIN

template<typename TInterpolator, typename TFragmentShader, typename TDepthTester, typename TOutputMerger>
class TriangleRasterizer
{
    const TInterpolator   &interpolator_;
    const TFragmentShader &fragment_shader_;
    const TDepthTester    &depth_tester_;
    const TOutputMerger   &output_merger_;

    using Varying = typename TFragmentShader::Input;
    using Pixel   = typename TFragmentShader::Output;

    static_assert(is_varying_v<Varying>);

    static bool visible(const Vec4 &pos) noexcept
    {
        return std::abs(pos.x) <= pos.w && std::abs(pos.y) <= pos.w && std::abs(pos.z) <= pos.w;
    }

    struct PixelBBox
    {
        Vec2i min_pixel;
        Vec2i max_pixel;
    };

    PixelBBox find_pixel_bounding(const Vec2 &v0, const Vec2 &v1, const Vec2 &v2) const
    {
        real min_x_f = std::min(v0.x, std::min(v1.x, v2.x));
        real min_y_f = std::min(v0.y, std::min(v1.y, v2.y));
        real max_x_f = std::max(v0.x, std::max(v1.x, v2.x));
        real max_y_f = std::max(v0.y, std::max(v1.y, v2.y));

        int min_x = agz::math::clamp<int>(int(std::floor(min_x_f)), 0, framebuffer_size.x - 1);
        int min_y = agz::math::clamp<int>(int(std::floor(min_y_f)), 0, framebuffer_size.y - 1);
        int max_x = agz::math::clamp<int>(int(std::ceil(max_x_f)), 0, framebuffer_size.x - 1);
        int max_y = agz::math::clamp<int>(int(std::ceil(max_y_f)), 0, framebuffer_size.y - 1);

        return { { min_x, min_y }, { max_x, max_y } };
    }

    enum class ClipPlane
    {
        PosX, NegX,
        PosY, NegY,
        PosZ, NegZ,
        PosW
    };

    template<ClipPlane CLIP_PLANE>
    static bool is_inside(const Vec4 &pos) noexcept
    {
        if constexpr(CLIP_PLANE == ClipPlane::PosX)
            return pos.x <= pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::NegX)
            return pos.x >= -pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::PosY)
            return pos.y <= pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::NegY)
            return pos.y >= -pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::PosZ)
            return pos.z <= pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::NegZ)
            return pos.z >= -pos.w;
        return pos.w >= EPS;
    }

    template<ClipPlane CLIP_PLANE>
    static real inct_ratio(const Vec4 &last, const Vec4 &curr) noexcept
    {
        if(CLIP_PLANE == ClipPlane::PosW)
            return (last.w - EPS) / (last.w - curr.w);
        if(CLIP_PLANE == ClipPlane::PosX)
            return (last.w - last.x) / ((last.w - last.x) - (curr.w - curr.x));
        if(CLIP_PLANE == ClipPlane::NegX)
            return (last.w + last.x) / ((last.w + last.x) - (curr.w + curr.x));
        if(CLIP_PLANE == ClipPlane::PosY)
            return (last.w - last.y) / ((last.w - last.y) - (curr.w - curr.y));
        if(CLIP_PLANE == ClipPlane::NegY)
            return (last.w + last.y) / ((last.w + last.y) - (curr.w + curr.y));
        if(CLIP_PLANE == ClipPlane::PosZ)
            return (last.w - last.z) / ((last.w - last.z) - (curr.w - curr.z));
        return (last.w + last.z) / ((last.w + last.z) - (curr.w + curr.z));
    }

    template<ClipPlane CLIP_PLANE>
    int clip_with_plane(const Varying *input, int input_count, Varying *output) const noexcept
    {
        int output_count = 0;

        for(int i = 0; i < input_count; ++i)
        {
            const Varying &last = input[(i - 1 + input_count) % input_count];
            const Varying &curr = input[i];

            bool is_last_inside = is_inside<CLIP_PLANE>(last.agz_position);
            bool is_curr_inside = is_inside<CLIP_PLANE>(curr.agz_position);

            if(is_last_inside != is_curr_inside)
            {
                Varying &dest = output[output_count++];
                real ratio = inct_ratio<CLIP_PLANE>(last.agz_position, curr.agz_position);
                dest.agz_position = math::mix(last.agz_position, curr.agz_position, ratio);
                interpolator_.lerp(last, curr, ratio, &dest);
            }

            if(is_curr_inside)
                output[output_count++] = curr;
        }

        return output_count;
    }

    static constexpr int CLIPPED_TRIANGLE_MAX_COUNT = 12;

    int clip_triangle(const Varying *input, Varying *output) const noexcept
    {
        bool is_v0_visible = visible(input[0].agz_position);
        bool is_v1_visible = visible(input[1].agz_position);
        bool is_v2_visible = visible(input[2].agz_position);
        if(is_v0_visible && is_v1_visible && is_v2_visible)
        {
            output[0] = input[0];
            output[1] = input[1];
            output[2] = input[2];
            return 3;
        }

        int varying_count = 3;
        Varying toutput2[CLIPPED_TRIANGLE_MAX_COUNT];
        varying_count = clip_with_plane<ClipPlane::PosW>(input, varying_count, output);
        varying_count = clip_with_plane<ClipPlane::PosX>(output, varying_count, toutput2);
        varying_count = clip_with_plane<ClipPlane::NegX>(toutput2, varying_count, output);
        varying_count = clip_with_plane<ClipPlane::PosY>(output, varying_count, toutput2);
        varying_count = clip_with_plane<ClipPlane::NegY>(toutput2, varying_count, output);
        varying_count = clip_with_plane<ClipPlane::PosZ>(output, varying_count, toutput2);
        varying_count = clip_with_plane<ClipPlane::NegZ>(toutput2, varying_count, output);

        assert(varying_count >= 3);
        return varying_count;
    }

public:

    using Interpolator   = TInterpolator;
    using FragmentShader = TFragmentShader;
    using DepthTester    = TDepthTester;
    using OutputMerger   = TOutputMerger;
    using Input          = Varying;

    static constexpr size_t VertexBatchSize = 3;

    Vec2i framebuffer_size;

    TriangleRasterizer(
        const Interpolator   &interpolator,
        const FragmentShader &fragment_shader,
        const DepthTester    &depth_tester,
        const OutputMerger   &output_merger,
        const Vec2i &framebuffer_size = Vec2i(0, 0)) noexcept
        : interpolator_   (interpolator),
          fragment_shader_(fragment_shader),
          depth_tester_   (depth_tester),
          output_merger_  (output_merger),
          framebuffer_size(framebuffer_size)
    {
        
    }

    void process(const Varying *varyings) const
    {
        Varying interpolated_varying;
        Pixel pixel_output;

        // clipping

        Varying clipped_varyings[CLIPPED_TRIANGLE_MAX_COUNT];
        real clipped_ws[CLIPPED_TRIANGLE_MAX_COUNT];
        int vertex_count = clip_triangle(varyings, clipped_varyings);

        // to pixel coordinates

        for(int i = 0; i < vertex_count; ++i)
        {
            real w = clipped_varyings[i].agz_position.w;
            clipped_ws[i] = w;
            clipped_varyings[i].agz_position /= w;
            clipped_varyings[i].agz_position.x = real(0.5) * (clipped_varyings[i].agz_position.x + 1) * framebuffer_size.x;
            clipped_varyings[i].agz_position.y = real(0.5) * (clipped_varyings[i].agz_position.y + 1) * framebuffer_size.y;
        }

        int vertex_end = vertex_count - 2;
        for(int i = 0; i < vertex_end; ++i)
        {
            int i0 = 0, i1 = i + 1, i2 = i + 2;

            Varying triangle_varyings[3] = { clipped_varyings[i0], clipped_varyings[i1], clipped_varyings[i2] };

            real w0 = clipped_ws[i0];
            real w1 = clipped_ws[i1];
            real w2 = clipped_ws[i2];

            // triangle bounding box

            Vec2 v0 = triangle_varyings[0].agz_position.xy();
            Vec2 v1 = triangle_varyings[1].agz_position.xy();
            Vec2 v2 = triangle_varyings[2].agz_position.xy();

            auto [min_pixel, max_pixel] = find_pixel_bounding(v0, v1, v2);

            // construct edge function

            Vec2 v1_v0 = v1 - v0;
            Vec2 v2_v0 = v2 - v0;

            bool top_left_01 = (v1.y > v0.y) || (v1.y == v0.y && v1.x > v0.x);
            bool top_left_12 = (v2.y > v1.y) || (v2.y == v1.y && v2.x > v1.x);
            bool top_left_20 = (v0.y > v2.y) || (v0.y == v2.y && v0.x > v2.x);

            real det = v1_v0.x * v2_v0.y - v2_v0.x * v1_v0.y;
            if(std::abs(det) < EPS)
                return;

            // triversal pixels

            for(int y = min_pixel.y; y <= max_pixel.y; ++y)
            {
                real pixel_centre_y = y + real(0.5);
                for(int x = min_pixel.x; x <= max_pixel.x; ++x)
                {
                    Vec2 pixel_centre(x + real(0.5), pixel_centre_y);

                    Vec2 pixel_v0 = pixel_centre - v0;
                    real alpha = (pixel_v0.x * v2_v0.y - v2_v0.x * pixel_v0.y) / det;
                    real beta = (v1_v0.x * pixel_v0.y - pixel_v0.x * v1_v0.y) / det;
                    real gamma = 1 - alpha - beta;

                    bool cond_beta = beta > 0 || (beta == 0 && top_left_01);
                    bool cond_alpha = alpha > 0 || (alpha == 0 && top_left_20);
                    bool cond_gamma = gamma > 0 || (gamma == 0 && top_left_12);
                    if(!cond_alpha || !cond_beta || !cond_gamma)
                        continue;

                    real corrected_alpha = w0 * w1 * alpha / (w1 * w2 + w2 * beta  * (w0 - w1) + w1 * alpha * (w0 - w2));
                    real corrected_beta  = w0 * w2 * beta / (w1 * w2 + w1 * alpha * (w0 - w2) + w2 * beta  * (w0 - w1));
                    real corrected_gamma = 1 - corrected_alpha - corrected_beta;

                    real weights[3]           = { gamma, alpha, beta };
                    real corrected_weights[3] = { corrected_gamma, corrected_alpha, corrected_beta };

                    // interpolation postprocess

                    interpolated_varying.agz_depth = corrected_weights[0] * triangle_varyings[0].agz_position.z
                                                   + corrected_weights[1] * triangle_varyings[1].agz_position.z
                                                   + corrected_weights[2] * triangle_varyings[2].agz_position.z;

                    interpolator_.process(triangle_varyings, weights, corrected_weights, &interpolated_varying);

                    // early depth test

                    if constexpr(DepthTester::early_depth_test)
                    {
                        if(!depth_tester_.process(x, y, interpolated_varying.agz_depth))
                            continue;
                    }

                    // fragment shader

                    if(!fragment_shader_.process(interpolated_varying, &pixel_output))
                        continue;

                    // depth test

                    if constexpr(!DepthTester::early_depth_test)
                    {
                        if(depth_tester_.process(x, y, interpolated_varying.agz_depth))
                            output_merger_.process(x, y, pixel_output);
                    }
                    else
                        output_merger_.process(x, y, pixel_output);
                }
            }
        }

    }
};

AGZ_RASTERIZER_END
