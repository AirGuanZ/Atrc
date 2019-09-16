#pragma once

#include <agz/rasterizer/common.h>
#include <agz/rasterizer/varying.h>

AGZ_RASTERIZER_BEGIN

template<typename TInterpolator, typename TFragmentShader, typename TOutputMerger>
class TriangleRasterizer
{
    static bool in_ndc(const Vec4 &pos) noexcept
    {
        return -1 <= pos.x && pos.x <= 1
            && -1 <= pos.y && pos.y <= 1
            &&  0 <= pos.z && pos.z <= 1;
    }

    const TInterpolator   *interpolator_;
    const TFragmentShader *fragment_shader_;
    const TOutputMerger   *output_merger_;

    using Varying = typename TFragmentShader::Input;
    using Pixel   = typename TFragmentShader::Output;

    static_assert(is_varying_v<Varying>);

public:

    using Interpolator   = TInterpolator;
    using FragmentShader = TFragmentShader;
    using OutputMerger   = TOutputMerger;
    using Input          = Varying;
    using Frame          = typename OutputMerger::Frame;

    static constexpr size_t VertexBatchSize = 3;

    TriangleRasterizer(
        const Interpolator *interpolator,
        const FragmentShader *fragment_shader,
        const OutputMerger *output_merger) noexcept
        : interpolator_(interpolator),
          fragment_shader_(fragment_shader),
          output_merger_(output_merger)
    {
        
    }

    void process(Varying *varyings, Frame *framebuffer) const
    {
        Varying interpolated_varying;
        Pixel pixel_output;

        // ndc coordinates

        real w0 = varyings[0].agz_position.w;
        real w1 = varyings[1].agz_position.w;
        real w2 = varyings[2].agz_position.w;

        varyings[0].agz_position /= w0;
        varyings[1].agz_position /= w1;
        varyings[2].agz_position /= w2;

        if(!in_ndc(varyings[0].agz_position) || !in_ndc(varyings[1].agz_position) || !in_ndc(varyings[2].agz_position))
            return;

        // ndc to pixel coordinates

        int fb_w = framebuffer->width();
        int fb_h = framebuffer->height();

        varyings[0].agz_position.x = real(0.5) * (varyings[0].agz_position.x + 1) * fb_w;
        varyings[1].agz_position.x = real(0.5) * (varyings[1].agz_position.x + 1) * fb_w;
        varyings[2].agz_position.x = real(0.5) * (varyings[2].agz_position.x + 1) * fb_w;

        varyings[0].agz_position.y = real(0.5) * (varyings[0].agz_position.y + 1) * fb_h;
        varyings[1].agz_position.y = real(0.5) * (varyings[1].agz_position.y + 1) * fb_h;
        varyings[2].agz_position.y = real(0.5) * (varyings[2].agz_position.y + 1) * fb_h;

        // preprocess interpolation

        interpolator_->preprocess(&varyings[0]);
        interpolator_->preprocess(&varyings[1]);
        interpolator_->preprocess(&varyings[2]);

        // triangle bounding box

        Vec2 v0 = varyings[0].agz_position.xy();
        Vec2 v1 = varyings[1].agz_position.xy();
        Vec2 v2 = varyings[2].agz_position.xy();

        real min_x_f = std::min(v0.x, std::min(v1.x, v2.x));
        real min_y_f = std::min(v0.y, std::min(v1.y, v2.y));
        real max_x_f = std::max(v0.x, std::max(v1.x, v2.x));
        real max_y_f = std::max(v0.y, std::max(v1.y, v2.y));

        int min_x = agz::math::clamp<int>(int(std::floor(min_x_f)), 0, fb_w - 1);
        int min_y = agz::math::clamp<int>(int(std::floor(min_y_f)), 0, fb_h - 1);
        int max_x = agz::math::clamp<int>(int(std::ceil(max_x_f)), 0, fb_w - 1);
        int max_y = agz::math::clamp<int>(int(std::ceil(max_y_f)), 0, fb_h - 1);

        // construct edge function

        Vec2 v1_v0 = v1 - v0;
        Vec2 v2_v1 = v2 - v1;
        Vec2 v0_v2 = v0 - v2;
        Vec2 v2_v0 = v2 - v0;

        Vec2 edge_w_01 = Vec2(-v1_v0.y, v1_v0.x);
        Vec2 edge_w_12 = Vec2(-v2_v1.y, v2_v1.x);
        Vec2 edge_w_20 = Vec2(-v0_v2.y, v0_v2.x);

        bool top_left_01 = (v1.y > v0.y) || (v1.y == v0.y && v1.x > v0.x);
        bool top_left_12 = (v2.y > v1.y) || (v2.y == v1.y && v2.x > v1.x);
        bool top_left_20 = (v0.y > v2.y) || (v0.y == v2.y && v0.x > v2.x);

        real det = v1_v0.x * v2_v0.y - v2_v0.x * v1_v0.y;
        if(std::abs(det) < EPS)
            return;

        // triversal pixels

        for(int y = min_y; y <= max_y; ++y)
        {
            real pixel_centre_y = y + real(0.5);
            for(int x = min_x; x <= max_x; ++x)
            {
                Vec2 pixel_centre(x + real(0.5), pixel_centre_y);

                Vec2 a_v0 = pixel_centre - v0;
                Vec2 a_v1 = pixel_centre - v1;
                Vec2 a_v2 = pixel_centre - v2;

                real edge01 = dot(edge_w_01, a_v0);
                real edge12 = dot(edge_w_12, a_v1);
                real edge20 = dot(edge_w_20, a_v2);

                edge01 += (edge01 == 0 && top_left_01) ? -1 : 0;
                edge12 += (edge12 == 0 && top_left_12) ? -1 : 0;
                edge20 += (edge20 == 0 && top_left_20) ? -1 : 0;

                if(edge01 < 0 && edge12 < 0 && edge20 < 0)
                {
                    // compute pixel weights
                    
                    real alpha = (a_v0.x * v2_v0.y - v2_v0.x * a_v0.y) / det;
                    real beta  = (v1_v0.x * a_v0.y - a_v0.x * v1_v0.y) / det;
                    real gamma = 1 - alpha - beta;

                    real corrected_alpha = w0 * w1 * alpha / (w1 * w2 + w2 * beta  * (w0 - w1) + w1 * alpha * (w0 - w2));
                    real corrected_beta  = w0 * w2 * beta  / (w1 * w2 + w1 * alpha * (w0 - w2) + w2 * beta  * (w0 - w1));
                    real corrected_gamma = 1 - corrected_alpha - corrected_beta;

                    real weights[3]           = { gamma, alpha, beta };
                    real corrected_weights[3] = { corrected_gamma, corrected_alpha, corrected_beta };
                    
                    // interpolation postprocess

                    interpolator_->postprocess(varyings, weights, corrected_weights, &interpolated_varying);

                    interpolated_varying.agz_depth = corrected_weights[0] * varyings[0].agz_position.z
                                                   + corrected_weights[1] * varyings[1].agz_position.z
                                                   + corrected_weights[2] * varyings[2].agz_position.z;

                    // fragment shader

                    if(!fragment_shader_->process(interpolated_varying, &pixel_output))
                        continue;

                    // depth test

                    if(interpolated_varying.agz_depth < framebuffer->depth_buffer().at(y, x))
                    {
                        framebuffer->depth_buffer().at(y, x) = interpolated_varying.agz_depth;
                        output_merger_->process(pixel_output, framebuffer, x, y);
                    }
                }
            }
        }
    }
};

AGZ_RASTERIZER_END
