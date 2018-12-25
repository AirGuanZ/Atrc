#include <Atrc/SH2D/ProjectResult.h>

namespace Atrc::SH2D
{

bool LightProjectResult::Serialize(AGZ::BinarySerializer &s) const
{
    return s.Serialize(SHC) && s.Serialize(coefs);
}

bool LightProjectResult::Deserialize(AGZ::BinaryDeserializer &ds)
{
    return ds.Deserialize(SHC) && ds.Deserialize(coefs);
}

void LightProjectResult::Rotate(const Mat3 &rotateMat)
{
    std::vector<std::vector<Real>> channels(
        SPECTRUM_CHANNEL_COUNT, std::vector<Real>(SHC));

    for(int c = 0; c < SPECTRUM_CHANNEL_COUNT; ++c)
    {
        for(int32_t s = 0; s < SHC; ++s)
            channels[c][s] = coefs[s][c];
    }

    using RotateFunc = void(*)(const Mat3&, Real*);
    RotateFunc rotateFunc;

    switch(SHC)
    {
    case 0: rotateFunc = AGZ::Math::RotateSH_L0<Real>; break;
    case 1: rotateFunc = AGZ::Math::RotateSH_L1<Real>; break;
    case 2: rotateFunc = AGZ::Math::RotateSH_L2<Real>; break;
    case 3: rotateFunc = AGZ::Math::RotateSH_L3<Real>; break;
    case 4: rotateFunc = AGZ::Math::RotateSH_L4<Real>; break;
    default:
        throw Atrc::Exception("Invalid SHC value: " + Str8::From(SHC));
    }

    for(int i = 0; i < SPECTRUM_CHANNEL_COUNT; ++i)
        rotateFunc(rotateMat, channels[i].data());
    
    for(int c = 0; c < SPECTRUM_CHANNEL_COUNT; ++c)
    {
        for(int32_t s = 0; s < SHC; ++s)
            coefs[s][c] = channels[c][s];
    }
}

bool SceneProjectResult::Serialize(AGZ::BinarySerializer &s) const
{
    return s.Serialize(SHC)        &&
           s.Serialize(coefs)      &&
           s.Serialize(binaryMask) &&
           s.Serialize(albedoMap)  &&
           s.Serialize(normalMap);
}

bool SceneProjectResult::Deserialize(AGZ::BinaryDeserializer &ds)
{
    return ds.Deserialize(SHC)        &&
           ds.Deserialize(coefs)      &&
           ds.Deserialize(binaryMask) &&
           ds.Deserialize(albedoMap)  &&
           ds.Deserialize(normalMap);
}

Image ReconstructFromSH(int32_t SHC, const Image *sceneCoefs, const Spectrum *lightCoefs)
{
    AGZ_ASSERT(SHC >= 1 && sceneCoefs && lightCoefs);

    auto resolution = sceneCoefs[0].GetSize();
    Image ret(resolution[0], resolution[1]);

    for(uint32_t y = 0; y < ret.GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < ret.GetWidth(); ++x)
        {
            Spectrum pixel;
            for(int32_t i = 0; i < SHC; ++i)
                pixel += sceneCoefs[i](x, y) * lightCoefs[i];
            ret(x, y) = pixel;
        }
    }

    return ret;
}

} // namespace Atrc::SH2D
