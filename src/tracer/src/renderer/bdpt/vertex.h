#pragma once

#include <agz/tracer/utility/math.h>

AGZ_TRACER_BEGIN

class BDPTEndPoint
{
public:

    Vec3 pos;
    Vec3 nor;
    real pdf_area = 1;

    Spectrum f;
    real pdf_dir  = 1;
    bool is_delta = false;

    virtual ~BDPTEndPoint() = default;

    virtual std::pair<Spectrum, real> eval(const Vec3 &next_pos) const noexcept = 0;
};

class BDPTCamera : public BDPTEndPoint
{
public:


};

AGZ_TRACER_END
