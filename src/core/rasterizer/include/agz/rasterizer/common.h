#pragma once

#include <agz/common/math.h>
#include <agz/utility/texture.h>

#define AGZ_RASTERIZER_BEGIN namespace agz::ras {
#define AGZ_RASTERIZER_END   }

AGZ_RASTERIZER_BEGIN

using RGB  = math::tcolor3<real>;
using RGBA = math::tcolor4<real>;

template<typename Pixel>
using ImageBuffer = texture::texture2d_t<Pixel>;

using Vec2 = math::tvec2<real>;
using Vec3 = math::tvec3<real>;
using Vec4 = math::tvec4<real>;

AGZ_RASTERIZER_END
