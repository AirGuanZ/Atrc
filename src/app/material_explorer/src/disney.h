#pragma once

#include "./material.h"
#include "./opengl.h"
#include "./to_json.h"

class Disney : public Material
{
    vec3 base_color_;

    float metallic_;
    float roughness_;
    float specular_tint_;
    float anisotropic_;
    float sheen_;
    float sheen_tint_;
    float clearcoat_;
    float clearcoat_gloss_;

    float transmission_;
    float ior_;

    float transmission_roughness_;
    float trans_ax_;
    float trans_ay_;

    float ax_;
    float ay_;
    vec3 base_color_tint_;

    float clearcoat_roughness_;

    float sample_diffuse_pdf_;
    float sample_specular_pdf_;
    float sample_clearcoat_pdf_;
    float sample_transmission_pdf_;

    void compute_secondary_params()
    {
        float aspect = anisotropic_ > 0 ? std::sqrt(1 - 0.9f * anisotropic_) : 1.0f;
        ax_ = (std::max)(0.001f, roughness_ * roughness_ / aspect);
        ay_ = (std::max)(0.001f, roughness_ * roughness_ * aspect);

        trans_ax_ = (std::max)(0.001f, transmission_roughness_ * transmission_roughness_ / aspect);
        trans_ay_ = (std::max)(0.001f, transmission_roughness_ * transmission_roughness_ * aspect);

        clearcoat_roughness_ = agz::math::mix(0.1f, 0.01f, clearcoat_gloss_);

        float lum = agz::math::color3f(base_color_.x, base_color_.y, base_color_.z).lum();
        if(lum > 0)
            base_color_tint_ = base_color_ / lum;
        else
            base_color_tint_ = vec3(1);

        float A = (std::min)(0.8f, 1 - metallic_);
        float B = 1 - A;
        sample_diffuse_pdf_      = A * (1 - transmission_);
        sample_transmission_pdf_ = A * transmission_;
        sample_specular_pdf_     = B * 2 / (2 + clearcoat_);
        sample_clearcoat_pdf_    = B * clearcoat_ / (2 + clearcoat_);
    }

public:

    Disney()
    {
        base_color_      = vec3(1);
        metallic_        = 0;
        roughness_       = 0.2f;
        specular_tint_   = 0;
        anisotropic_     = 0;
        sheen_           = 0;
        sheen_tint_      = 0;
        clearcoat_       = 0;
        clearcoat_gloss_ = 1;
        transmission_    = 0;
        transmission_roughness_ = 0.2f;
        ior_             = 1.5f;
        compute_secondary_params();
    }

    std::string shader_source() const override
    {
        return agz::file::read_txt_file("./shader/disney.glsl");
    }

    bool show_gui() override
    {
        ImGui::PushID(this);
        AGZ_SCOPE_GUARD({ ImGui::PopID(); });

        bool ret = false;
        ret |= ImGui::ColorEdit3("base_color",       &base_color_[0]);
        ret |= ImGui::SliderFloat("metallic",        &metallic_, 0, 1);
        ret |= ImGui::SliderFloat("roughness",       &roughness_, 0, 1);
        ret |= ImGui::SliderFloat("specular_tint",   &specular_tint_, 0, 1);
        ret |= ImGui::SliderFloat("anisotropic",     &anisotropic_, 0, 1);
        ret |= ImGui::SliderFloat("sheen",           &sheen_, 0, 1);
        ret |= ImGui::SliderFloat("sheen_tint",      &sheen_tint_, 0, 1);
        ret |= ImGui::SliderFloat("clearcoat",       &clearcoat_, 0, 1);
        ret |= ImGui::SliderFloat("clearcoat_gloss", &clearcoat_gloss_, 0, 1);
        ret |= ImGui::SliderFloat("transmission",    &transmission_, 0, 1);
        ret |= ImGui::SliderFloat("transmission_roughness", &transmission_roughness_, 0, 1);
        ret |= ImGui::InputFloat("ior",              &ior_, 0.01f);
        ior_ = agz::math::clamp<float>(ior_, 1.001f, 10);

        if(ret)
            compute_secondary_params();
        return ret;
    }

    void set_shader_uniforms(program_t &prog) const override
    {
        prog.set_uniform_unchecked("base_color",              base_color_);
        prog.set_uniform_unchecked("base_color_tint",         base_color_tint_);
        prog.set_uniform_unchecked("metallic",                metallic_);
        prog.set_uniform_unchecked("roughness",               roughness_);
        prog.set_uniform_unchecked("specular_tint",           specular_tint_);
        prog.set_uniform_unchecked("anisotropic",             anisotropic_);
        prog.set_uniform_unchecked("sheen",                   sheen_);
        prog.set_uniform_unchecked("sheen_tint",              sheen_tint_);
        prog.set_uniform_unchecked("clearcoat",               clearcoat_);
        prog.set_uniform_unchecked("coearcoat_roughness",     clearcoat_roughness_);
        prog.set_uniform_unchecked("transmission",            transmission_);
        prog.set_uniform_unchecked("trans_ax",                trans_ax_);
        prog.set_uniform_unchecked("trans_ay",                trans_ay_);
        prog.set_uniform_unchecked("ior",                     ior_);
        prog.set_uniform_unchecked("ax",                      ax_);
        prog.set_uniform_unchecked("ay",                      ay_);
        prog.set_uniform_unchecked("sample_diffuse_pdf",      sample_diffuse_pdf_);
        prog.set_uniform_unchecked("sample_specular_pdf",     sample_specular_pdf_);
        prog.set_uniform_unchecked("sample_clearcoat_pdf",    sample_clearcoat_pdf_);
        prog.set_uniform_unchecked("sample_transmission_pdf", sample_transmission_pdf_);
    }

    std::string to_json() const override
    {
        std::string template_str = R"___(
{
    "type": "disney",
    "base_color": {
        "type": "constant",
        "texel": ${BASE_COLOR}
    },
    "metallic": {
        "type": "constant",
        "texel": ${METALLIC}
    },
    "roughness": {
        "type": "constant",
        "texel": ${ROUGHNESS}
    },
    "specular_tint": {
        "type": "constant",
        "texel": ${SPECULAR_TINT}
    },
    "anisotropic": {
        "type": "constant",
        "texel": ${ANISOTROPIC}
    },
    "sheen": {
        "type": "constant"
        "texel": ${SHEEN}
    },
    "sheen_tint": {
        "type": "constant",
        "texel": ${SHEEN_TINT}
    },
    "clearcoat": {
        "type": "constant",
        "texel": ${CLEARCOAT}
    },
    "clearcoat_gloss": {
        "type": "constant",
        "texel": ${CLEARCOAT_GLOSS}
    },
    "transmission": {
        "type": "constant",
        "texel": ${TRANSMISSION}
    },
    "transmission_roughness": {
        "type": "constant",
        "texel": ${TRANSMISSION_ROUGHNESS}
    },
    "ior": {
        "type": "constant",
        "texel": ${IOR}
    }
}
)___";
        agz::stdstr::replace_(template_str, "${BASE_COLOR}",             ::to_json(base_color_));
        agz::stdstr::replace_(template_str, "${METALLIC}",               ::to_json(metallic_));
        agz::stdstr::replace_(template_str, "${ROUGHNESS}",              ::to_json(roughness_));
        agz::stdstr::replace_(template_str, "${SPECULAR_TINT}",          ::to_json(specular_tint_));
        agz::stdstr::replace_(template_str, "${ANISOTROPIC}",            ::to_json(anisotropic_));
        agz::stdstr::replace_(template_str, "${SHEEN}",                  ::to_json(sheen_));
        agz::stdstr::replace_(template_str, "${SHEEN_TINT}",             ::to_json(sheen_tint_));
        agz::stdstr::replace_(template_str, "${CLEARCOAT}",              ::to_json(clearcoat_));
        agz::stdstr::replace_(template_str, "${CLEARCOAT_GLOSS}",        ::to_json(clearcoat_gloss_));
        agz::stdstr::replace_(template_str, "${TRANSMISSION}",           ::to_json(transmission_));
        agz::stdstr::replace_(template_str, "${TRANSMISSION_ROUGHNESS}", ::to_json(transmission_roughness_));
        agz::stdstr::replace_(template_str, "${IOR}",                    ::to_json(ior_));
        return template_str;
    }
};
