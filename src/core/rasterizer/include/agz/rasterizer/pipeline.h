#pragma once

#include <type_traits>

#include <agz/rasterizer/common.h>
#include <agz/rasterizer/framebuffer.h>
#include <agz/rasterizer/varying.h>
#include <agz/utility/misc.h>

AGZ_RASTERIZER_BEGIN

/*
concept VertexShader
    using Input = ?;
    using Output = ?;
    void process(in const Input&, out uninited Output*);
*/

/*
concept Interpolator
    using Input = ?;
    using Output = Input;
    void preprocess(inout Input*);
    void postprocess(in const Input[3], in const real[3], in const real[3], out uninited Output*);
*/

/*
concept Rasterizer<Interpolator, FragmentShader, OutputMerger>
    using Input = ?;
    void process(in Input[3], in const FragmentContinuation&);
*/

/*
concept FragmentShader
    using Input = ?;
    using Output = ?;
    // return: continue executing or not
    bool process(in const Input&, out uninited Output*);
*/

/*
concept DepthTester
    // return: continue executing or not
    bool process(inout float *old_depth, in float new_depth);
*/

/*
concept OutputMerger
    using Input = ?;
    using Frame = ?;
    void process(in const Input&, inout Frame*, int x, int y);
*/

template<typename VertexShader,
         typename Interpolator,
         typename Rasterizer,
         typename FragmentShader,
         typename OutputMerger>
class pipeline_type_checker : public std::true_type
{
    template<typename T>
    struct is_framebuffer_aux : std::false_type { };
    template<typename...PixelFormats>
    struct is_framebuffer_aux<Framebuffer<PixelFormats...>> : std::true_type { };

public:

    static_assert(
        std::is_base_of_v<VaryingBase, typename VertexShader::Output>,
        "VertexShader::Output must be derived from VaryingBase");
    static_assert(
        std::is_same_v<typename VertexShader::Output, typename Interpolator::Input>,
        "VertexShader::Output must be same with Interpolator::Input");
    static_assert(
        std::is_same_v<typename Interpolator::Input, typename Interpolator::Output>,
        "Interpolator::Input must be same with Interpolator::Output");
    static_assert(
        std::is_same_v<typename Interpolator::Output, typename Rasterizer::Input>,
        "Interpolator::Output must be same with Rasterizer::Input");
    static_assert(
        is_framebuffer_aux<typename OutputMerger::Frame>::value,
        "OutputMerger::Frame must be one of Framebuffer<...>");
    static_assert(
        std::is_same_v<typename Rasterizer::Input, typename FragmentShader::Input>,
        "Rasterizer::Output must be same with FragmentShader::Input");
    static_assert(
        std::is_same_v<typename FragmentShader::Output, typename OutputMerger::Input>,
        "FragmentShader::Output must be same with OutputMerger::Input");
};

template<typename VertexShader,
         typename Interpolator,
         typename Rasterizer,
         typename FragmentShader,
         typename OutputMerger>
constexpr bool pipeline_type_checker_v = pipeline_type_checker<
    VertexShader, Interpolator, Rasterizer, FragmentShader, OutputMerger>::value;

template<typename TVertexShader, typename TRasterizer>
struct Pipeline
{
    using VertexShader   = TVertexShader;
    using Rasterizer     = TRasterizer;
    using Interpolator   = typename Rasterizer::Interpolator;
    using FragmentShader = typename Rasterizer::FragmentShader;
    using OutputMerger   = typename Rasterizer::OutputMerger;

    static_assert(
        pipeline_type_checker_v<
            VertexShader,
            Interpolator,
            Rasterizer,
            FragmentShader,
            OutputMerger>);

    using Vertex  = typename VertexShader::Input;
    using Varying = typename VertexShader::Output;
    using Frame   = typename Rasterizer::Frame;

    Pipeline() noexcept
        : Pipeline(nullptr, nullptr)
    {
        
    }

    Pipeline(const VertexShader *vertex_shader, const Rasterizer *rasterizer) noexcept
        : vertex_shader(vertex_shader), rasterizer(rasterizer)
    {
        
    }

    const VertexShader *vertex_shader;
    const Rasterizer   *rasterizer;
};

template<typename VertexShader, typename Rasterizer>
void run_pipeline(
    const Pipeline<VertexShader, Rasterizer> &pipeline,
    typename Pipeline<VertexShader, Rasterizer>::Frame *framebuffer,
    const misc::span<const typename Pipeline<VertexShader, Rasterizer>::Vertex> vertices)
{
    constexpr size_t VertexBatchSize = Rasterizer::VertexBatchSize;
    assert(framebuffer);
    assert(vertices.size() % VertexBatchSize == 0);

    // prepare buffer for varying variables

    using Varying = typename VertexShader::Output;
    static_assert(is_varying_v<Varying>);
    Varying varyings[VertexBatchSize];

    for(size_t i = 0; i < vertices.size(); i += VertexBatchSize)
    {
        // execute vertex shader

        for(size_t j = 0; j < VertexBatchSize; ++j)
            pipeline.vertex_shader->process(vertices[i + j], &varyings[j]);

        // rasterize

        pipeline.rasterizer->process(varyings, framebuffer);
    }
}

AGZ_RASTERIZER_END
