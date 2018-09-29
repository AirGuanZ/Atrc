#pragma once

#include <type_traits>
#include <Utils.h>

#include "../Core/Spectrum.h"
#include "../Math/Geometry.h"
#include "../Sample/Sample.h"
#include "../Sample/SampleSequence.h"

AGZ_NS_BEG(Atrc)

enum class BxDFType : uint8_t
{
    Reflection   = 1 << 0,
    Transmission = 1 << 1,
    Diffuse      = 1 << 2,
    Glossy       = 1 << 3,
    Specular     = 1 << 4,
    Ambient      = 1 << 5,

    AllReflection   = Reflection    | Diffuse | Glossy | Specular,
    AllTransmission = Transmission  | Diffuse | Glossy | Specular,
    All             = AllReflection | AllTransmission
};

constexpr bool HasBxDFType(BxDFType typeSet, BxDFType typeTested)
{
    using UT = std::underlying_type_t<BxDFType>;
    return (static_cast<UT>(typeSet) &
            static_cast<UT>(typeTested))
           != 0;
}

template<typename T>
struct IsBxDFTypeAux
{
    static constexpr bool value = std::is_same_v<T, BxDFType>;
};

template<typename...T,
         std::enable_if_t<AGZ::TypeOpr::All_v<
                IsBxDFTypeAux, T...>, int> = 0>
constexpr BxDFType CombineBxDFTypes(T...types)
{
    return static_cast<BxDFType>(
        (... | static_cast<std::underlying_type_t<T>>(types)));
}

struct BxDFSample
{
    Vec3r dir;      // Surface local坐标系中的采样方向
    Spectrum coef;
    Real pdf;
};

ATRC_INTERFACE BxDF
{
public:

    virtual ~BxDF() = default;

    virtual BxDFType GetType() const = 0;

    virtual Spectrum Eval(
        const SurfaceLocal &sl, const Vec3r &wi, const Vec3r &wo) const = 0;

    virtual Option<BxDFSample> Sample(
        const SurfaceLocal &sl, const Vec3r &wo, SampleSeq2D &samSeq,
        BxDFType type) const = 0;

    virtual Real PDF(
        const SurfaceLocal &sl, const Vec3r &samDir, const Vec3r &wo) const = 0;

    virtual Spectrum AmbientRadiance() const
    {
        return SPECTRUM::BLACK;
    }
};

ATRC_INTERFACE Material
{
public:

    virtual ~Material() = default;

    virtual BxDF *GetBxDF(const SurfaceLocal &sl) const = 0;
};

AGZ_NS_END(Atrc)
