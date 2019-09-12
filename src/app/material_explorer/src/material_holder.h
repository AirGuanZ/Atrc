#pragma once

#include "./camera.h"
#include "./envir.h"
#include "./material.h"
#include "./opengl.h"

class MaterialHolder : public agz::misc::uncopyable_t
{
    program_t prog_;
    program_t fullscreen_;
    vertex_array_t vao_;

    std::unique_ptr<Material> mat_;

    struct Framebuffer
    {
        framebuffer_t fbo;
        texture2d_t color;
        renderbuffer_t depth;
    };

    static std::unique_ptr<Framebuffer> create_framebuffer(int width, int height);

    std::unique_ptr<Framebuffer> last_frame_;
    std::unique_ptr<Framebuffer> this_frame_;

    GLint frame_counter_;
    GLint start_seed_;
    int max_spp_;
    vec2 pixel_low;
    vec2 pixel_delta;

    void init_prog(const std::string &mat_src);

public:

    MaterialHolder(std::unique_ptr<Material> &&mat, int framebuffer_width, int framebuffer_height);

    bool show_gui(GLFWwindow *window);

    void render(const Camera &camera, const EnvironmentLightManager &env);

    void restart();
};
