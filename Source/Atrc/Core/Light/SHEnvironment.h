#pragma once

#include <vector>

#include <Atrc/Core/Light/InfiniteLight.h>

namespace Atrc
{
    
class SHEnvLight : public InfiniteLight
{
    int SHOrder_;
    std::vector<Spectrum> SHCoefs_;

public:

    SHEnvLight(int SHOrder, const Spectrum *coefs, Real rotateDeg);

    Spectrum NonAreaLe(const Ray &r) const noexcept override;
};

} // namespace Atrc
