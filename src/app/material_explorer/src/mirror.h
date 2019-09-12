#pragma once

#include "./material.h"
#include "./opengl.h"

class Mirror : public Material
{
    vec3 color_ = vec3(1);

public:

    std::string shader_source() const override
    {
        return R"___(
uniform vec3 mirror_color;

bool sample_bsdf(in Intersection inct, out vec3 wi, out vec3 coef, inout uint sample_state)
{
    vec3 lwo = normalize(global_to_local(inct.coord, inct.wr));
    if(lwo.z <= 0)
        return false;
    vec3 lwi = reflect(lwo, vec3(0, 0, 1));
    wi = local_to_global(inct.coord, lwi);
    coef = mirror_color;
    return true;
}
)___";
    }

    bool show_gui() override
    {
        return ImGui::ColorEdit3("color", &color_[0]);
    }

    void set_shader_uniforms(program_t &prog) const override
    {
        prog.set_uniform_unchecked("mirror_color", color_);
    }

    std::string to_json() const override
    {
        std::string template_str = R"___(
{
    "type": "mirror",
    "rc_map": {
      "type": "constant",
      "texel": ${COLOR}
    },
    "fresnel": {
      "type": "always_one"
    }
}
)___";
        agz::stdstr::replace_(template_str, "${COLOR}", ::to_json(color_));
        return template_str;
    }
};
