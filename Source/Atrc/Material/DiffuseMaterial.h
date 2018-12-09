#pragma once

#include <Atrc/Core/Core.h>
#include <Atrc/Material/NormalMapper.h>

AGZ_NS_BEG(Atrc)

// ÒÑÖ§³Önormal mapping
class DiffuseMaterial : public Material
{
    Spectrum color_;
    const NormalMapper *norMap_;

public:

    DiffuseMaterial(const Spectrum &albedo, const NormalMapper *norMap);

    void Shade(const SurfacePoint &sp, ShadingPoint *dst, AGZ::ObjArena<> &arena) const override;
};

AGZ_NS_END(Atrc)
