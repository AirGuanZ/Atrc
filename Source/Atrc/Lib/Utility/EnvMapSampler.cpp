#include <Atrc/Lib/Utility/EnvMapSampler.h>

namespace Atrc
{

namespace
{
    Real ComputeTexelWeight(Real deltaU, Real v0, Real v1) noexcept(noexcept(Sin(std::declval<Real>())))
    {
        Real deltaPhi = 2 * PI * deltaU;
        Real theta0 = PI * v0 - PI / 2;
        Real theta1 = PI * v1 - PI / 2;
        return Abs(deltaPhi * (Sin(theta1) - Sin(theta0)));
    }

    std::optional<std::pair<Real, Real>> IntersectSegment(const std::pair<Real, Real> &a, const std::pair<Real, Real> &b) noexcept
    {
        AGZ_ASSERT(a.first <= a.second && b.first <= b.second);
        auto p = std::make_pair(Max(a.first, b.first), Min(a.second, b.second));
        if(p.first > p.second)
            return std::nullopt;
        return p;
    }
}

EnvMapSampler::EnvMapSampler(const AGZ::Texture2D<Color3f> &tex, uint32_t distribWidth, uint32_t distribHeight)
{
    AGZ_ASSERT(distribWidth > 0 && distribWidth > 0);

    uint32_t texWidth = tex.GetWidth(), texHeight = tex.GetHeight();

    // 将texture转换为distribWidth x distribHeight大小的采样亮度图

    AGZ::Texture2D<Real> resizedTex(distribWidth, distribHeight);

    for(uint32_t disY = 0; disY < distribHeight; ++disY)
    {
        Real disVBegin = Max<Real>(0, (disY - Real(0.5)) / distribHeight);
        Real disVEnd   = Min<Real>(0, (disY + Real(1.5)) / distribHeight);

        uint32_t texYBegin = Clamp<uint32_t>(
            uint32_t(std::floor(disVBegin * texHeight)), 0, texHeight);
        uint32_t texYEnd   = Clamp<uint32_t>(
            uint32_t(std::ceil(disVEnd * texHeight) + 1), texYBegin, texHeight);

        for(uint32_t disX = 0; disX < distribWidth; ++disX)
        {
            Real disUBegin = Max<Real>(0, (disX - Real(0.5)) / distribWidth);
            Real disUEnd   = Min<Real>(0, (disX + Real(1.5)) / distribWidth);

            uint32_t texXBegin = Clamp<uint32_t>(
                uint32_t(std::floor(disUBegin * texWidth)), 0, texWidth);
            uint32_t texXEnd   = Clamp<uint32_t>(
                uint32_t(std::ceil(disUEnd * texWidth) + 1), texXBegin, texWidth);

            Real texel = 0, weight = 0;
            for(uint32_t texY = texYBegin; texY < texYEnd; ++texY)
            {
                Real tv0 = Real(texY) / distribHeight;
                Real tv1 = Real(texY + 1) / distribHeight;

                auto vp = IntersectSegment({ disVBegin, disVEnd }, { tv0, tv1 });
                if(!vp)
                    continue;

                for(uint32_t texX = texXBegin; texX < texXEnd; ++texX)
                {
                    Real tu0 = Real(texX) / distribWidth;
                    Real tu1 = Real(texX + 1) / distribWidth;

                    auto up = IntersectSegment({ disUBegin, disUEnd }, { tu0, tu1 });
                    if(!up)
                        continue;
                    
                    Real w = ComputeTexelWeight(up->second - up->first, vp->first, vp->second);
                    Spectrum c = tex(texX, texY);
                    texel  += w * (c.r + c.g + c.b);
                    weight += w;
                }
            }

            Real value = weight ? texel / weight : Real(0);
            resizedTex(disX, disY) = value;
        }
    }

    // 在pdf的每个texel上叠加一个pdfOffset，并转换为未归一化的cdf

    // TODO
}

} // namespace Atrc
