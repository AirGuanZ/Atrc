#pragma once

#include "./opengl.h"

class Camera : public agz::misc::uncopyable_t
{
    float vert_rad_;
    float hori_rad_;
    float aspect_;

    struct CameraParams
    {
        vec3 eye;
        vec3 film_left_bottom;
        vec3 film_x;
        vec3 film_y;
    } camera_params_;

    void update_camera_params();

public:

    explicit Camera(float aspect);

    void set_shader_uniforms(program_t &prog) const;

    bool show_gui();

    bool update(float ms);
};
