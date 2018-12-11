#pragma once

#include <Atrc/Lib/Core/Camera.h>

namespace Atrc
{

class Sampler
{
public:

    virtual ~Sampler() = default;

    virtual std::unique_ptr<Sampler> Clone(int seed) const = 0;

    virtual void StartPixel(const Vec2i &pixel) = 0;

    virtual bool NextSample() = 0;

    virtual Camera::CameraSample GetCameraSample() = 0;

    virtual Real GetSample() = 0;

    virtual Vec2 GetSample2D() = 0;
};

} // namespace Atrc
