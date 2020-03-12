#pragma once

#include <agz/tracer/create/texture2d.h>
#include <agz/factory/factory.h>

AGZ_TRACER_FACTORY_BEGIN

namespace helper
{
    
    inline RC<Texture2D> child_texture_or_constant(
        CreatingContext &context, const ConfigGroup &params,
        const std::string &name, real texel)
    {
        if(auto node = params.find_child_group(name))
            return context.create<Texture2D>(*node);
        return create_constant2d_texture({}, texel);
    }

} // namespace helper

AGZ_TRACER_FACTORY_END
