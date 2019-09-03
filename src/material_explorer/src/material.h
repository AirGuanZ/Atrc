#pragma once

#include <agz/utility/misc.h>

class Material : agz::misc::uncopyable_t
{
public:

    virtual ~Material() = default;

    virtual std::string shader_source() const = 0;

    virtual void set_shader_uniforms() const = 0;

    virtual bool show_gui() = 0;
};
