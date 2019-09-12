#pragma once

#include <agz/utility/string.h>

#include "./material.h"
#include "./opengl.h"
#include "./to_json.h"

class Diffuse : public Material
{
    vec3 albedo_ = vec3(1);

public:

    std::string shader_source() const override
    {
        return R"___(
uniform vec3 albedo;

void zweighted_on_hemisphere(out vec3 dir, out float pdf, inout uint sample_state)
{
    float u = next_sample(sample_state);
    float v = next_sample(sample_state);
    u = 2 * u - 1;
    v = 2 * v - 1;

    vec2 sam;
    if(u != 0 || v != 0)
    {
        float theta, r;
        if(abs(u) > abs(v))
        {
            r = u;
            theta = 0.25 * PI * (v / u);
        }
        else
        {
            r = v;
            theta = 0.5 * PI - 0.25 * PI * (u / v);
        }
        sam = r * vec2(cos(theta), sin(theta));
    }

    float z = sqrt(max(0.0, 1 - dot(sam, sam)));
    dir = vec3(sam, z);
    pdf = z / PI;
}

bool sample_bsdf(in Intersection inct, out vec3 wi, out vec3 coef, inout uint sample_state)
{
    vec3 lwo = normalize(global_to_local(inct.coord, inct.wr));
    if(lwo.z <= 0)
        return false;
    vec3 lwi; float pdf;
    zweighted_on_hemisphere(lwi, pdf, sample_state);
    if(lwi.z <= 0 || pdf < EPS)
        return false;
    wi = local_to_global(inct.coord, lwi);
    coef = albedo / PI * lwi.z / pdf;
    return true;
}
)___";
    }

    bool show_gui() override
    {
        return ImGui::ColorEdit3("albedo", &albedo_[0]);
    }

    void set_shader_uniforms(program_t &prog) const override
    {
        prog.set_uniform_unchecked("albedo", albedo_);
    }

    std::string to_json() const override
    {
        std::string template_str = R"___(
{
    "type": "ideal_diffuse",
    "albedo": {
        "type": "constant",
        "texel": ${ALBEDO_TEXEL}
    }
}
)___";
        agz::stdstr::replace_(template_str, "${ALBEDO_TEXEL}", ::to_json(albedo_));
        return template_str;
    }
};
