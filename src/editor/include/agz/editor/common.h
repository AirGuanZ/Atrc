#pragma once

#include <QColor>

#include <agz/tracer/common.h>

#define AGZ_EDITOR_BEGIN namespace agz::editor {
#define AGZ_EDITOR_END   }

AGZ_EDITOR_BEGIN

using tracer::real;

using tracer::PI_r;

using tracer::Vec2;
using tracer::Vec3;
using tracer::Vec4;

using tracer::Vec2i;
using tracer::Vec3i;

using tracer::Rect2;
using tracer::Rect2i;

using tracer::Mat3;
using tracer::Mat4;

using Vec3d = math::vec3d;

using tracer::Spectrum;

using tracer::Image2D;
using tracer::Image3D;

inline Spectrum qcolor_to_spectrum(const QColor &color) noexcept
{
    return {
        real(color.redF()),
        real(color.greenF()),
        real(color.blueF())
    };
}

inline QColor spectrum_to_qcolor(const Spectrum &spec) noexcept
{
    const Spectrum s = spec.saturate();
    return QColor::fromRgbF(s.r, s.g, s.b);
}

using tracer::RC;
using tracer::Box;
using tracer::newRC;
using tracer::newBox;

AGZ_EDITOR_END
