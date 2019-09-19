#include <filesystem>
#include <iostream>

#include <agz/rasterizer/depth_tester.h>
#include <agz/rasterizer/pipeline.h>
#include <agz/rasterizer/rasterizer.h>
#include <agz/utility/image.h>
#include <agz/utility/mesh.h>
#include <agz/utility/system.h>
#include <agz/utility/time.h>

void save_color_buffer(const std::string &filename, const agz::ras::ImageBuffer<agz::ras::RGB> &buffer, bool open_after_save = true)
{
    assert(buffer.is_available());

    agz::ras::ImageBuffer<agz::ras::RGB> flipped(buffer.height(), buffer.width());
    for(int y = 0; y < flipped.height(); ++y)
    {
        for(int x = 0; x < flipped.width(); ++x)
        {
            int ty = flipped.height() - 1 - y;
            flipped(y, x) = buffer(ty, x);
        }
    }

    auto imgu8 = flipped.map(&agz::math::to_color3b<agz::real>);
    auto abs_filename = absolute(std::filesystem::path(filename)).string();
    agz::img::save_rgb_to_png_file(abs_filename, imgu8.raw_data(), imgu8.width(), imgu8.height());
    if(open_after_save)
        agz::sys::open_with_default_app(abs_filename);
}

void run()
{
    using namespace agz;
    using namespace ras;

    struct Vertex
    {
        Vec3 pos;
        Vec3 nor;
    };

    struct Varying : VaryingBase
    {
        Vec3 normal;
    };

    struct VertexShader
    {
        using Input  = Vertex;
        using Output = Varying;

        Mat4 WVP;
        Mat4 world;

        void process(const Input &v, Output *output) const noexcept
        {
            output->agz_position = WVP * Vec4(v.pos.x, v.pos.y, v.pos.z, 1);
            output->normal = (world * Vec4(v.nor.x, v.nor.y, v.nor.z, 0)).xyz();
        }
    };

    struct Interpolator
    {
        using Input  = Varying;
        using Output = Varying;

        void lerp(const Input &a, const Input &b, real t, Output *output) const
        {
            output->normal = ((1 - t) * a.normal + t * b.normal).normalize();
        }

        void process(const Input *input, const real *weight, const real *corrected_weight, Output *output) const
        {
            output->normal = corrected_weight[0] * input[0].normal
                           + corrected_weight[1] * input[1].normal
                           + corrected_weight[2] * input[2].normal;
        }
    };

    struct FragmentShader
    {
        using Input  = Varying;
        using Output = RGB;

        bool process(const Input &input, Output *output) const
        {
            Vec3 light_dir(1, 1, -2);
            float light_factor = std::max(0.0f, dot(-light_dir.normalize(), input.normal.normalize()));
            *output = RGB(light_factor);
            return true;
        }
    };

    struct OutputMerger
    {
        using Input = RGB;

        ImageBuffer<RGB> *color_buffer;

        void process(int x, int y, const Input &input) const
        {
            color_buffer->at(y, x) = input;
        }
    };

    constexpr int FB_W = 1024, FB_H = 768;
    ImageBuffer<RGB>  color_buffer(FB_H, FB_W);
    ImageBuffer<real> depth_buffer(FB_H, FB_W, 1);

    VertexShader       vertex_shader;
    Interpolator       interpolator;
    FragmentShader     fragment_shader;
    DefaultDepthTester depth_tester  = { &depth_buffer };
    OutputMerger       output_merger = { &color_buffer };

    vertex_shader.world = Mat4::scale(Vec3(0.06f))
                        * Mat4::rotate_x(math::deg2rad(90.0f));
    vertex_shader.WVP = Mat4::perspective(math::deg2rad(30.0f), float(FB_W) / FB_H, 0.01f, 100.0f)
                      * Mat4::look_at({ -2, -10, 2 }, { 0, 0, 0 }, { 0, 0, 1 })
                      * vertex_shader.world;

    TriangleRasterizer rasterizer(interpolator, fragment_shader, depth_tester, output_merger);
    rasterizer.framebuffer_size = { FB_W, FB_H };
    Pipeline pipeline(vertex_shader, rasterizer);

    auto mesh = mesh::load_from_file("./bunny.obj");
    std::vector<Vertex> vertices(mesh.size() * 3);
    for(size_t i = 0, j = 0; i < vertices.size(); i += 3, ++j)
    {
        vertices[i].pos     = mesh[j].vertices[0].position;
        vertices[i + 1].pos = mesh[j].vertices[1].position;
        vertices[i + 2].pos = mesh[j].vertices[2].position;
        vertices[i].nor     = mesh[j].vertices[0].normal;
        vertices[i + 1].nor = mesh[j].vertices[1].normal;
        vertices[i + 2].nor = mesh[j].vertices[2].normal;
    }
    mesh.clear();

    time::clock_t clock;
    clock.restart();
    color_buffer.clear({});
    depth_buffer.clear(1);
    run_pipeline(pipeline, misc::span<Vertex>(vertices.data(), vertices.size()));
    std::cout << clock.us() / 1000.0f << "ms" << std::endl;

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
