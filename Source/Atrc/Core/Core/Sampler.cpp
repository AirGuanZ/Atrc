#include <Atrc/Core/Core/Sampler.h>

namespace Atrc
{
    
Vec2 Sampler::GetReal2()
{
    Real u = GetReal();
    Real v = GetReal();
    return { u, v };
}

Vec3 Sampler::GetReal3()
{
    Vec2 uv = GetReal2();
    Real w = GetReal();
    return { uv, w };
}

} // namespace Atrc
