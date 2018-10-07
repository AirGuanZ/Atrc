#pragma once

#include <Atrc/Common.h>

AGZ_NS_BEG(Atrc::CommonSampler)

struct Uniform_InUnitSphere
{
    struct Result
    {
        Vec3r sample;
        Real pdf;
    };

    static Result Sample();

    static Real PDF(const Vec3r &sample);
};

struct ZWeighted_OnUnitHemisphere
{
    struct Result
    {
        Vec3r sample;
        Real pdf;
    };

    static Result Sample();

    static Real PDF(const Vec3r &sample);
};

struct Uniform_OnHemisphere
{
    struct Result
    {
        Vec3r sample;
        Real pdf;
    };

    static Result Sample();

    static Real PDF(const Vec3r &sample);
};

AGZ_NS_END(Atrc::CommonSampler)
