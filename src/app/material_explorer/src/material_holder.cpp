#include <iostream>

#include <agz/utility/string.h>

#include "./material_holder.h"

std::unique_ptr<MaterialHolder::Framebuffer> MaterialHolder::create_framebuffer(int width, int height)
{
    auto ret = std::make_unique<Framebuffer>();
    ret->fbo.initialize_handle();
    
    ret->color.initialize_handle();
    ret->color.initialize_format(1, GL_RGBA32F, width, height);
    ret->fbo.attach(GL_COLOR_ATTACHMENT0, ret->color);

    ret->depth.initialize_handle();
    ret->depth.set_format(GL_DEPTH_COMPONENT32, width, height);
    ret->fbo.attach(GL_DEPTH_ATTACHMENT, ret->depth);

    return ret;
}

void MaterialHolder::init_prog(const std::string &mat_src)
{
    static const std::string vertex_shader_src = R"___(
#version 450 core
out vec2 screen_coord;
void main(void)
{
    float x = (gl_VertexID == 2 || gl_VertexID == 4 || gl_VertexID == 5) ? 1.0 : -1.0;
    float y = (gl_VertexID == 1 || gl_VertexID == 2 || gl_VertexID == 4) ? 1.0 : -1.0;
    screen_coord = vec2(x, y);
    gl_Position = vec4(x, y, 0.5, 1.0);
}
)___";

    const std::string fragment_shader_src = agz::file::read_txt_file("./shader/frag.glsl");
    prog_ = program_t::build_from(
        vertex_shader_t::from_memory(vertex_shader_src),
        fragment_shader_t::from_memory(fragment_shader_src + mat_src));

    static const std::string full_screen_fragment_src = R"___(
#version 450 core
uniform sampler2D tex;
in vec2 screen_coord;
layout(location = 0) out vec4 pixel_color;
void main(void)
{
    vec2 uv = 0.5 * screen_coord + vec2(0.5);
    vec3 color = texture(tex, uv).rgb;
    const float gamma = 1 / 2.2;
    color.r = pow(color.r, gamma);
    color.g = pow(color.g, gamma);
    color.b = pow(color.b, gamma);
    pixel_color = vec4(color, 1);
}
)___";
    fullscreen_ = program_t::build_from(
        vertex_shader_t::from_memory(vertex_shader_src),
        fragment_shader_t::from_memory(full_screen_fragment_src));
}

MaterialHolder::MaterialHolder(std::unique_ptr<Material> &&mat, int framebuffer_width, int framebuffer_height)
    : mat_(std::move(mat)), frame_counter_(0), start_seed_(0), max_spp_(10000)
{
    init_prog(mat_->shader_source());
    vao_ = vertex_array_t(true);

    last_frame_ = create_framebuffer(framebuffer_width, framebuffer_height);
    last_frame_->color.set_param(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    last_frame_->color.set_param(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    this_frame_ = create_framebuffer(framebuffer_width, framebuffer_height);
    this_frame_->color.set_param(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    this_frame_->color.set_param(GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    pixel_low = vec2(-0.5f / framebuffer_width, -0.5f / framebuffer_height);
    pixel_delta = vec2(1.0f / framebuffer_width, 1.0f / framebuffer_height);
}

bool MaterialHolder::show_gui(GLFWwindow *window)
{
    ImGui::PushID(this);
    AGZ_SCOPE_GUARD({ ImGui::PopID(); });
    bool ret = false;

    if(ImGui::Button("to clipboard") && mat_)
    {
        auto str = agz::stdstr::trim(mat_->to_json());
        std::cout << str << std::endl;
        glfwSetClipboardString(window, str.c_str());
    }

    ImGui::SameLine();

    if(frame_counter_ > std::numeric_limits<int>::max() / 2)
        frame_counter_ = max_spp_;
    ImGui::Text("spp: %d", std::min(frame_counter_, max_spp_));

    ImGui::Separator();

    if(mat_->show_gui())
    {
        restart();
        ret = true;
    }

    return ret;
}

void MaterialHolder::render(const Camera &camera, const EnvironmentLightManager &env)
{
    this_frame_->fbo.bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog_.bind();
    vao_.bind();

    camera.set_shader_uniforms(prog_);
    env.set_shader_uniforms(prog_);

    prog_.set_uniform_unchecked("last_frame", 1);
    last_frame_->color.bind(1);

    prog_.set_uniform_unchecked("frame_counter", frame_counter_++);
    prog_.set_uniform_unchecked("start_seed",    start_seed_);
    prog_.set_uniform_unchecked("max_spp",       max_spp_);
    prog_.set_uniform_unchecked("pixel_low",     pixel_low);
    prog_.set_uniform_unchecked("pixel_delta",   pixel_delta);

    mat_->set_shader_uniforms(prog_);

    prog_.set_uniform_unchecked("sphere_radius", 1.0f);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    vao_.unbind();
    prog_.unbind();

    this_frame_->fbo.unbind();

    fullscreen_.bind();
    vao_.bind();

    fullscreen_.set_uniform_unchecked("tex", 0);
    this_frame_->color.bind(0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    vao_.unbind();
    fullscreen_.unbind();

    std::swap(this_frame_, last_frame_);
}

void MaterialHolder::restart()
{
    frame_counter_ = 0;
    start_seed_ = (start_seed_ + 1) % 100000;
}
