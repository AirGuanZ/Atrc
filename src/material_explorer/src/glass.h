#pragma once

#include "./material.h"
#include "./opengl.h"

class Glass : public Material
{
    vec3 color_ = vec3(1);
    float ior_ = 1.5f;

public:

    std::string shader_source() const override
    {
        return R"___(
uniform vec3 color;
uniform float ior;

bool refract(in vec3 nwo, in vec3 nor, in float eta, out vec3 wi)
{
    float cos_theta_i = abs(nwo.z);
    float sin_theta_i2 = max(0.0, 1 - cos_theta_i * cos_theta_i);
    float sin_theta_t2 = eta * eta * sin_theta_i2;
    if(sin_theta_t2 >= 1)
        return false;
    float cos_theta_t = sqrt(1 - sin_theta_t2);
    wi = normalize((eta * cos_theta_i - cos_theta_t) * nor - eta * nwo);
    return true;
}

float fresnel(in float eta, in float cos_theta_i)
{
    float eta_i = eta, eta_o = 1;
    if(cos_theta_i < 0)
    {
        eta_i = 1;
        eta_o = eta;
        cos_theta_i = -cos_theta_i;
    }
    
    float sin_theta_i = sqrt(max(0.0, 1 - cos_theta_i * cos_theta_i));
    float sin_theta_t = sin_theta_i * eta_o / eta_i;
    if(sin_theta_t >= 1)
        return 1;

    float cos_theta_t = sqrt(max(0.0, 1 - sin_theta_t * sin_theta_t));
    float para = (eta_i * cos_theta_i - eta_o * cos_theta_t) / (eta_i * cos_theta_i + eta_o * cos_theta_t);
    float perp = (eta_o * cos_theta_i - eta_i * cos_theta_t) / (eta_o * cos_theta_i + eta_i * cos_theta_t);

    return 0.5 * (para * para + perp * perp);
}

bool sample_bsdf(in Intersection inct, out vec3 wi, out vec3 coef, inout uint sample_state)
{
    vec3 nwo = normalize(global_to_local(inct.coord, inct.wr));
    vec3 nor = nwo.z > 0 ? vec3(0, 0, 1) : vec3(0, 0, -1);

    float fr = fresnel(ior, nwo.z);
    if(next_sample(sample_state) < fr)
    {
        vec3 lwi = vec3(-nwo.x, -nwo.y, nwo.z);
        wi = local_to_global(inct.coord, lwi);
        coef = color;
    }

    float eta_i = nwo.z > 0 ? 1 : ior;
    float eta_t = nwo.z > 0 ? ior : 1;
    float eta = eta_i / eta_t;

    vec3 lwi;
    if(!refract(nwo, nor, eta, lwi))
        return false;
    
    wi = local_to_global(inct.coord, lwi);
    coef = eta * eta * color;
    return true;
}
)___";
    }

    bool show_gui() override
    {
        bool ret = false;
        ret |= ImGui::ColorEdit3("color", &color_[0]);
        ret |= ImGui::InputFloat("ior", &ior_, 0.01f);
        ior_ = agz::math::clamp<float>(ior_, 1.001f, 10);
        return ret;
    }

    void set_shader_uniforms(program_t &prog) const override
    {
        prog.set_uniform_unchecked("color", color_);
        prog.set_uniform_unchecked("ior", ior_);
    }

    std::string to_json() const override
    {
        std::string template_str = R"___(
{
    "type": "glass",
    "fresnel": {
      "type": "dielectric",
      "eta_out": {
        "type": "constant",
        "texel": [ 1 ]
      },
      "eta_in": {
        "type": "constant",
        "texel": ${IOR}
      }
    },
    "color_map": {
      "type": "constant",
      "texel": ${COLOR}
    }
}
)___";
        agz::stdstr::replace_(template_str, "${IOR}", ::to_json(vec3(ior_)));
        agz::stdstr::replace_(template_str, "${COLOR}", ::to_json(vec3(color_)));
        return template_str;
    }
};
