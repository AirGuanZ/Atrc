#pragma once

#include <Utils/Serialize.h>

#include <Atrc/Lib/Core/TFilm.h>
#include <Atrc/SH2D/LightProjector.h>
#include <Atrc/SH2D/SceneProjector.h>

namespace Atrc::SH2D
{

struct LightProjectResult
{
    int32_t SHC;
    std::vector<Spectrum> coefs; // assert SHC == coefs.size()

    bool Serialize(AGZ::BinarySerializer &s) const;
    bool Deserialize(AGZ::BinaryDeserializer &ds);

    void Rotate(const Mat3 &rotateMat);
};

struct SceneProjectResult
{
    int32_t SHC;
    std::vector<Image> coefs;
    AGZ::Texture2D<Real> binaryMask;
    Image albedoMap;
    Image normalMap;

    bool Serialize(AGZ::BinarySerializer &s) const;
    bool Deserialize(AGZ::BinaryDeserializer &ds);
};

} // namespace Atrc::SH2D
