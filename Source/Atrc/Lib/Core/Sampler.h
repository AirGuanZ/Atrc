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

    virtual Real GetReal() = 0;

    virtual Vec2 GetReal2() = 0;

    static int Real2Int(Real real, int begin, int end) noexcept;
};

// ================================= Implementation

inline int Sampler::Real2Int(Real real, int begin, int end) noexcept
{
    int delta = end - begin;
    return begin + (std::min<int>)(real * delta, delta - 1);
}

} // namespace Atrc
