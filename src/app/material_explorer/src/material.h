#pragma once

#include "./opengl.h"

class Material : agz::misc::uncopyable_t
{
public:

    virtual ~Material() = default;

    virtual std::string shader_source() const = 0;

    virtual void set_shader_uniforms(program_t &prog) const = 0;

    virtual bool show_gui() = 0;

    virtual std::string to_json() const = 0;
};
