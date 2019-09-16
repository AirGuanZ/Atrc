#include <filesystem>
#include <iostream>

#include <agz/rasterizer/pipeline.h>
#include <agz/rasterizer/rasterizer.h>
#include <agz/utility/image.h>
#include <agz/utility/system.h>

AGZ_RASTERIZER_BEGIN

void save_color_buffer(const std::string &filename, const ImageBuffer<RGB> &buffer)
{
    assert(buffer.is_available());
    auto abs_filename = absolute(std::filesystem::path(filename)).string();
    auto imgu8 = buffer.map(&agz::math::to_color3b<agz::real>);
    agz::img::save_rgb_to_png_file(abs_filename, imgu8.raw_data(), imgu8.width(), imgu8.height());
    agz::sys::open_with_default_app(abs_filename);
}

void draw_line(ImageBuffer<RGB> &buffer, int x0, int y0, int x1, int y1, const RGB &color)
{
    bool steep = false;

    if(std::abs(x0 - x1) < std::abs(y0 - y1))
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }

    if(x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0, dy = y1 - y0;
    int derr = 2 * std::abs(dy);
    int err = 0;
    int y = y0;
    if(steep)
    {
        if(y1 > y0)
        {
            for(int x = x0; x <= x1; ++x)
            {
                buffer(x, y) = color;
                err += derr;
                if(err > dx)
                {
                    ++y;;
                    err -= 2 * dx;
                }
            }
        }
        else
        {
            for(int x = x0; x <= x1; ++x)
            {
                buffer(x, y) = color;
                err += derr;
                if(err > dx)
                {
                    --y;
                    err -= 2 * dx;
                }
            }
        }
    }
    else
    {
        if(y1 > y0)
        {
            for(int x = x0; x <= x1; ++x)
            {
                buffer(y, x) = color;
                err += derr;
                if(err > dx)
                {
                    ++y;
                    err -= 2 * dx;
                }
            }
        }
        else
        {
            for(int x = x0; x <= x1; ++x)
            {
                buffer(y, x) = color;
                err += derr;
                if(err > dx)
                {
                    --y;
                    err -= 2 * dx;
                }
            }
        }
    }
}

void draw_triangle(ImageBuffer<RGB> &buffer, const Vec2 &v0, const Vec2 &v1, const Vec2 &v2, const RGB &color)
{
    // compute bounding box

    float min_x_f = std::min(v0.x, std::min(v1.x, v2.x));
    float min_y_f = std::min(v0.y, std::min(v1.y, v2.y));
    float max_x_f = std::max(v0.x, std::max(v1.x, v2.x));
    float max_y_f = std::max(v0.y, std::max(v1.y, v2.y));

    int min_x = agz::math::clamp<int>(int(std::floor(min_x_f)), 0, buffer.width() - 1);
    int min_y = agz::math::clamp<int>(int(std::floor(min_y_f)), 0, buffer.height() - 1);
    int max_x = agz::math::clamp<int>(int(std::ceil(max_x_f)), 0, buffer.width() - 1);
    int max_y = agz::math::clamp<int>(int(std::ceil(max_y_f)), 0, buffer.height() - 1);

    // construct edge function

    Vec2 v1_v0 = v1 - v0;
    Vec2 v2_v1 = v2 - v1;
    Vec2 v0_v2 = v0 - v2;

    Vec2 edge_factor_01 = Vec2(-v1_v0.y, v1_v0.x);
    Vec2 edge_factor_12 = Vec2(-v2_v1.y, v2_v1.x);
    Vec2 edge_factor_20 = Vec2(-v0_v2.y, v0_v2.x);

    bool top_left_01 = (v1.y > v0.y) || ((v1.y == v0.y && v1.x > v0.x));
    bool top_left_12 = (v2.y > v1.y) || ((v2.y == v1.y && v2.x > v1.x));
    bool top_left_20 = (v0.y > v2.y) || ((v0.y == v2.y && v0.x > v2.x));

    // triversal pixels in bbox

    for(int y = min_y; y <= max_y; ++y)
    {
        for(int x = min_x; x <= max_x; ++x)
        {
            Vec2 pixel_centre(x + 0.5f, y + 0.5f);
            
            real edge_value_01 = dot(edge_factor_01, pixel_centre - v0);
            real edge_value_12 = dot(edge_factor_12, pixel_centre - v1);
            real edge_value_20 = dot(edge_factor_20, pixel_centre - v2);

            edge_value_01 += (edge_value_01 == 0 && top_left_01) ? -1 : 0;
            edge_value_12 += (edge_value_12 == 0 && top_left_12) ? -1 : 0;
            edge_value_20 += (edge_value_20 == 0 && top_left_20) ? -1 : 0;

            if(edge_value_01 < 0 && edge_value_12 < 0 && edge_value_20 < 0)
                buffer(y, x) += color;
        }
    }
}

AGZ_RASTERIZER_END

void run()
{
    using namespace agz;
    using namespace ras;

    using Vertex = Vec2;

    struct Varying : VaryingBase { };

    struct VertexShader
    {
        using Input  = Vertex;
        using Output = Varying;

        void process(const Input &v, Output *output) const noexcept
        {
            output->agz_position = Vec4(v.x, v.y, real(0.5), 1);
        }
    };

    struct Interpolator
    {
        using Input  = Varying;
        using Output = Varying;

        void preprocess(Input *input) const
        {
            
        }

        void postprocess(const Input *input, const real *weight, const real *corrected_weight, Output *output) const
        {
            
        }
    };

    struct FragmentShader
    {
        using Input  = Varying;
        using Output = RGB;

        bool process(const Input &input, Output *output) const
        {
            *output = RGB(0, 0.5f, 0.5f);
            return true;
        }
    };

    struct OutputMerger
    {
        using Input = RGB;
        using Frame = Framebuffer<RGB>;

        void process(const Input &input, Frame *frame, int x, int y) const
        {
            frame->color_buffer<0>().at(y, x) += input;
        }
    };

    VertexShader   vertex_shader;
    Interpolator   interpolator;
    FragmentShader fragment_shader;
    OutputMerger   output_merger;

    using Rasterizer = TriangleRasterizer<Interpolator, FragmentShader, OutputMerger>;

    TriangleRasterizer rasterizer(&interpolator, &fragment_shader, &output_merger);
    Pipeline<VertexShader, Rasterizer> pipeline(&vertex_shader, &rasterizer);

    constexpr int FB_W = 640, FB_H = 480;
    ImageBuffer<RGB>  color_buffer(FB_H, FB_W);
    ImageBuffer<real> depth_buffer(FB_H, FB_W, 1);
    Framebuffer<RGB>  framebuffer(&color_buffer, &depth_buffer);

    Vertex vertices[] =
    {
        { -1, -1 },
        { -1, 1 },
        { 1, 1 },
        { -1, -1 },
        { 1, 1 },
        { 1, -1 }
    };
    run_pipeline(pipeline, &framebuffer, misc::span<const Vertex>(vertices, 6));

    save_color_buffer("./output.png", color_buffer);
}

int main()
{
    try
    {
        run();
    }
    catch(const std::exception &err)
    {
        std::cout << err.what() << std::endl;
        return -1;
    }
}
