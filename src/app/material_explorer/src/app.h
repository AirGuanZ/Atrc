#pragma once

#include <agz/utility/misc.h>
#include <agz/utility/time.h>

#include "./camera.h"
#include "./envir.h"
#include "./material_holder.h"

class App : public agz::misc::uncopyable_t
{
    std::unique_ptr<MaterialHolder> mat_;

    std::vector<std::string> material_names_;
    std::string current_material_name_;

    EnvironmentLightManager env_light_;

    int fb_width_;
    int fb_height_;

    agz::time::clock_t clock_;
    Camera camera_;

public:

    App(int framebuffer_width, int framebuffer_height);

    void update(GLFWwindow *window);

    void render();
};
