#include "./camera.h"

void Camera::update_camera_params()
{
    constexpr float sensor_width = 0.2f;
    const float sensor_height = sensor_width / aspect_;

    const float cos_theta = std::cos(vert_rad_);
    vec3 pos(std::cos(hori_rad_) * cos_theta, std::sin(hori_rad_) * cos_theta, std::sin(vert_rad_));
    pos *= 6;

    const vec3 dst(0, 0, 0);
    const vec3 up(0, 0, 1);

    const vec3 dir = (dst - pos).normalize();
    const vec3 film_centre = pos + dist_ * dir;

    const vec3 x_dir = cross(dir, up).normalize();
    const vec3 y_dir = cross(x_dir, dir).normalize();
    const float x_len = sensor_width;
    const float y_len = sensor_height;
    const vec3 x_ori = x_len * x_dir;
    const vec3 y_ori = y_len * y_dir;

    camera_params_.eye = pos;
    camera_params_.film_left_bottom = film_centre - 0.5f * x_ori - 0.5f * y_ori;
    camera_params_.film_x = x_ori;
    camera_params_.film_y = y_ori;
}

Camera::Camera(float aspect)
    : vert_rad_(0), hori_rad_(0), aspect_(aspect), dist_(0.15f)
{
    update_camera_params();
}

void Camera::set_shader_uniforms(program_t &prog) const
{
    prog.set_uniform_unchecked("eye_pos",          camera_params_.eye);
    prog.set_uniform_unchecked("film_left_bottom", camera_params_.film_left_bottom);
    prog.set_uniform_unchecked("film_x_ori",       camera_params_.film_x);
    prog.set_uniform_unchecked("film_y_ori",       camera_params_.film_y);
}

bool Camera::show_gui()
{
    ImGui::PushID(this);
    AGZ_SCOPE_GUARD({ ImGui::PopID(); });
    bool ret = false;
    ret |= ImGui::InputFloat("hori", &hori_rad_, 0, 0, "%.6f");
    ret |= ImGui::InputFloat("vert", &vert_rad_, 0, 0, "%.6f");
    ret |= ImGui::SliderFloat("dist", &dist_, 0.03f, 0.5f);
    if(ret)
        update_camera_params();
    return ret;
}

bool Camera::update(float ms)
{
    constexpr float HORI_SPEED = 0.001f;
    constexpr float VERT_SPEED = 0.001f;
    constexpr float VERT_LIMIT = agz::math::PI_f / 2 - 0.01f;

    bool ret = false;
    
    if(ImGui::GetIO().KeysDown[int('A')])
    {
        hori_rad_ -= HORI_SPEED * ms;
        ret = true;
    }

    if(ImGui::GetIO().KeysDown[int('D')])
    {
        hori_rad_ += HORI_SPEED * ms;
        ret = true;
    }

    if(ImGui::GetIO().KeysDown[int('W')])
    {
        vert_rad_ += VERT_SPEED * ms;
        if(vert_rad_ > VERT_LIMIT)
            vert_rad_ = VERT_LIMIT;
        ret = true;
    }

    if(ImGui::GetIO().KeysDown[int('S')])
    {
        vert_rad_ -= VERT_SPEED * ms;
        if(vert_rad_ < -VERT_LIMIT)
            vert_rad_ = -VERT_LIMIT;
        ret = true;
    }

    if(ret)
        update_camera_params();
    return ret;
}
