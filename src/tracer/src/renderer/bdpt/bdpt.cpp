#pragma once

#include <agz/tracer/core/renderer.h>

AGZ_TRACER_BEGIN

namespace bdpt
{

class BDPTEndPoint
{
public:

    Vec3 pos;
    Vec3 nor;
    real pdf_area = 0;

    Spectrum f;
    real pdf_dir  = 0;
    bool is_delta = false;

    virtual ~BDPTEndPoint() = default;

    // return (emission, pdf)
    virtual std::pair<Spectrum, real> eval(const Vec3 &next_pos) const noexcept = 0;
};

class Vertex
{
public:

    Vec3 pos;
    Vec3 wr;
    Vec3 wsam;

    Spectrum f; // eval(wsam, wr)

    real pdf_proj_radiance   = 0;
    real pdf_proj_importance = 0;
    bool is_delta       = false;
};

class BDPT : public Renderer
{
public:


};

} // namespace bdpt

AGZ_TRACER_END
