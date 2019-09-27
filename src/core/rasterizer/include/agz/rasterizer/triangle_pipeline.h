#pragma once

#include <agz/rasterizer/rasterizer_scheduler.h>
#include <agz/rasterizer/triangle_clipper.h>
#include <agz/rasterizer/varying.h>

AGZ_RASTERIZER_BEGIN

template<
    typename TVertexShader,
    typename TInterpolator,
    typename TFragmentShader,
    typename TDepthTester,
    typename TOutputMerger>
class TrianglePipeline
{
    using Varying = typename TFragmentShader::Input;
    using Pixel   = typename TFragmentShader::Output;
    using Clipper = TriangleClipper<TInterpolator, Varying>;

    using Self = TrianglePipeline<TVertexShader, TInterpolator, TFragmentShader, TDepthTester, TOutputMerger>;

    static_assert(is_varying_v<Varying>);
    
    struct ClippedTriangle
    {
        Varying varyings[3];
        real w[3] = { 0, 0, 0 };
    };

    using VaryingDataBatch = std::vector<ClippedTriangle>;

    class Worker : public RasterizerScheduler::ThreadWorker<VaryingDataBatch>
    {
    public:

        const Self *pipeline = nullptr;

        void run(const RasterizerScheduler::ThreadLocalData &thread_local_data, const VaryingDataBatch &data) const noexcept override
        {
            return pipeline->rasterization_worker_func(thread_local_data, data);
        }
    };

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

        int min_x = math::clamp<int>(int(std::floor(min_x_f)), 0, framebuffer_size_.x - 1);
        int min_y = math::clamp<int>(int(std::floor(min_y_f)), 0, framebuffer_size_.y - 1);
        int max_x = math::clamp<int>(int(std::ceil(max_x_f)), 0, framebuffer_size_.x - 1);
        int max_y = math::clamp<int>(int(std::ceil(max_y_f)), 0, framebuffer_size_.y - 1);

        return { { min_x, min_y }, { max_x, max_y } };
    }

    void execute_vertex_shader(const typename TVertexShader::Input *vertices, Varying *output, size_t vertex_count) const
    {
        for(size_t i = 0; i < vertex_count; ++i)
            vertex_shader_->process(vertices[i], &output[i]);
    }

    static constexpr int CLIPPED_TRIANGLE_MAX_COUNT = Clipper::CLIPPED_TRIANGLE_MAX_COUNT;

    void rasterize(Varying *varyings, size_t varying_count) const
    {
        auto triangle_batch = std::make_shared<VaryingDataBatch>();

        for(size_t i = 0; i < varying_count; i += 3)
        {
            static thread_local Varying clipped_varyings[CLIPPED_TRIANGLE_MAX_COUNT];
            static thread_local real clipped_ws[CLIPPED_TRIANGLE_MAX_COUNT];
            int clipped_vertex_count = clipper_.clip(&varyings[i], clipped_varyings);
            if(clipped_vertex_count < 3)
                continue;

            for(int j = 0; j < clipped_vertex_count; ++j)
            {
                real w = clipped_varyings[j].agz_position.w;
                clipped_ws[j] = w;
                clipped_varyings[j].agz_position /= w;
                clipped_varyings[j].agz_position.x = real(0.5) * (clipped_varyings[j].agz_position.x + 1) * framebuffer_size_.x;
                clipped_varyings[j].agz_position.y = real(0.5) * (clipped_varyings[j].agz_position.y + 1) * framebuffer_size_.y;
            }

            int vertex_end = clipped_vertex_count - 2;
            for(int j = 0; j < vertex_end; ++j)
            {
                int i0 = 0, i1 = j + 1, i2 = j + 2;
                triangle_batch->push_back({
                    { clipped_varyings[i0], clipped_varyings[i1], clipped_varyings[i2] },
                    { clipped_ws[i0],       clipped_ws[i1],       clipped_ws[i2]       }
                });

                if(triangle_batch->size() >= rasterizer_batch_size_)
                {
                    scheduler_->add_task<VaryingDataBatch>(worker_, std::move(triangle_batch));
                    triangle_batch = std::make_shared<VaryingDataBatch>();
                }
            }
        }

        if(!triangle_batch->empty())
            scheduler_->add_task<VaryingDataBatch>(worker_, std::move(triangle_batch));
    }

    void rasterization_worker_func(const RasterizerScheduler::ThreadLocalData &thread_local_data, const VaryingDataBatch &data) const
    {
        Varying interpolated_varying;
        Pixel pixel_output;

        for(auto &triangle : data)
        {
            auto &triangle_varyings = triangle.varyings;

            real w0 = triangle.w[0];
            real w1 = triangle.w[1];
            real w2 = triangle.w[2];

            Vec2 v0 = triangle_varyings[0].agz_position.xy();
            Vec2 v1 = triangle_varyings[1].agz_position.xy();
            Vec2 v2 = triangle_varyings[2].agz_position.xy();

            // triangle bounding box

            auto [min_pixel, max_pixel] = find_pixel_bounding(v0, v1, v2);

            // top-left edge

            Vec2 v1_v0 = v1 - v0;
            Vec2 v2_v0 = v2 - v0;

            bool top_left_01 = (v1.y > v0.y) || (v1.y == v0.y && v1.x > v0.x);
            bool top_left_12 = (v2.y > v1.y) || (v2.y == v1.y && v2.x > v1.x);
            bool top_left_20 = (v0.y > v2.y) || (v0.y == v2.y && v0.x > v2.x);

            real det = v1_v0.x * v2_v0.y - v2_v0.x * v1_v0.y;
            if(det == 0)
                return;

            // traversal pixels

            int y_begin = min_pixel.y;
            while((y_begin % thread_local_data.thread_count) != thread_local_data.thread_index)
                ++y_begin;

            for(int y = y_begin; y <= max_pixel.y; y += thread_local_data.thread_count)
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
                    real corrected_beta = w0 * w2 * beta / (w1 * w2 + w1 * alpha * (w0 - w2) + w2 * beta  * (w0 - w1));
                    real corrected_gamma = 1 - corrected_alpha - corrected_beta;

                    // interpolation postprocess

                    interpolated_varying.agz_depth = corrected_gamma * triangle_varyings[0].agz_position.z
                                                   + corrected_alpha * triangle_varyings[1].agz_position.z
                                                   + corrected_beta * triangle_varyings[2].agz_position.z;

                    Varying t_varying;
                    t_varying.agz_depth = interpolated_varying.agz_depth;
                    if(gamma > EPS)
                    {
                        real lerp_weight = alpha / (gamma + alpha);
                        real corr_lerp_weight = corrected_alpha / (corrected_gamma + corrected_alpha);
                        interpolator_->process(triangle_varyings[0], triangle_varyings[1], lerp_weight, corr_lerp_weight, &t_varying);
                        interpolator_->process(t_varying, triangle_varyings[2], beta, corrected_beta, &interpolated_varying);
                    }
                    else
                    {
                        real lerp_weight = beta / (alpha + beta);
                        real corr_lerp_weight = corrected_beta / (corrected_alpha + corrected_beta);
                        interpolator_->process(triangle_varyings[1], triangle_varyings[2], lerp_weight, corr_lerp_weight, &t_varying);
                        interpolator_->process(t_varying, triangle_varyings[0], gamma, corrected_gamma, &interpolated_varying);
                    }

                    // early depth test

                    if constexpr(DepthTester::early_depth_test)
                    {
                        if(!depth_tester_->process(x, y, interpolated_varying.agz_depth))
                            continue;
                    }

                    // fragment shader

                    if(!fragment_shader_->process(interpolated_varying, &pixel_output))
                        continue;

                    // depth test

                    if constexpr(!DepthTester::early_depth_test)
                    {
                        if(depth_tester_->process(x, y, interpolated_varying.agz_depth))
                            output_merger_->process(x, y, pixel_output);
                    }
                    else
                        output_merger_->process(x, y, pixel_output);
                }
            }
        }
    }

    const TVertexShader   *vertex_shader_;
    const TInterpolator   *interpolator_;
    const TFragmentShader *fragment_shader_;
    const TDepthTester    *depth_tester_;
    const TOutputMerger   *output_merger_;

    RasterizerScheduler *scheduler_;

    Clipper clipper_;

    Vec2i framebuffer_size_;

    size_t vertex_shader_batch_size_;
    size_t rasterizer_batch_size_;

    std::shared_ptr<Worker> worker_;

public:

    using VertexShader   = TVertexShader;
    using FragmentShader = TFragmentShader;
    using Interpolator   = TInterpolator;
    using DepthTester    = TDepthTester;
    using OutputMerger   = TOutputMerger;

    using Vertex = typename VertexShader::Input;

    TrianglePipeline()
    {
        vertex_shader_   = nullptr;
        interpolator_    = nullptr;
        fragment_shader_ = nullptr;
        depth_tester_    = nullptr;
        output_merger_   = nullptr;

        scheduler_       = nullptr;

        vertex_shader_batch_size_ = 1024 * 3;
        rasterizer_batch_size_    = 512;

        worker_ = std::make_shared<Worker>();
        worker_->pipeline = this;
    }

    TrianglePipeline(
        const VertexShader   *vertex_shader,
        const Interpolator   *interpolator,
        const FragmentShader *fragment_shader,
        const DepthTester    *depth_tester,
        const OutputMerger   *output_merger)
    {
        vertex_shader_   = vertex_shader;
        interpolator_    = interpolator;
        fragment_shader_ = fragment_shader;
        depth_tester_    = depth_tester;
        output_merger_   = output_merger;

        scheduler_ = nullptr;

        vertex_shader_batch_size_ = 1024 * 3;
        rasterizer_batch_size_ = 512;

        worker_ = std::make_shared<Worker>();
        worker_->pipeline = this;
    }

    void process(const Vertex *vertices, size_t vertex_count) const
    {
        assert(vertex_count && vertex_count % 3 == 0);
        assert(vertex_shader_batch_size_ && vertex_shader_batch_size_ % 3 == 0);

        std::vector<Varying> varyings(vertex_shader_batch_size_);
        while(vertex_count > 0)
        {
            size_t batch_size = (std::min)(vertex_shader_batch_size_, vertex_count);
            execute_vertex_shader(vertices, varyings.data(), batch_size);
            rasterize(varyings.data(), batch_size);
            vertices += batch_size;
            vertex_count -= batch_size;
        }
    }

    void set_vertex_shader(const VertexShader *vertex_shader)
    {
        vertex_shader_ = vertex_shader;
    }

    void set_fragment_shader(const FragmentShader *fragment_shader)
    {
        fragment_shader_ = fragment_shader;
    }

    void set_interpolator(const Interpolator *interpolator)
    {
        interpolator_ = interpolator;
        clipper_.interpolator = interpolator;
    }

    void set_depth_tester(const DepthTester *depth_tester)
    {
        depth_tester_ = depth_tester;
    }

    void set_output_merger(const OutputMerger *output_merger)
    {
        output_merger_ = output_merger;
    }

    void set_scheduler(RasterizerScheduler *scheduler)
    {
        scheduler_ = scheduler;
    }

    void set_framebuffer_size(const Vec2i &framebuffer_size)
    {
        framebuffer_size_ = framebuffer_size;
    }

    void set_vertex_shader_batch_size(size_t size)
    {
        assert(size);
        vertex_shader_batch_size_ = size * 3;
    }

    void set_rasterizer_batch_size(size_t size)
    {
        assert(size);
        rasterizer_batch_size_ = size;
    }
};

AGZ_RASTERIZER_END
