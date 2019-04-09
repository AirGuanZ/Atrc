#pragma once

#include <Atrc/Editor/Fresnel/Fresnel.h>
#include <Atrc/Editor/GL.h>

namespace Atrc::Editor
{

class Conductor : public ResourceCommonImpl<IFresnel, Conductor>
{
    Vec3f etaOut_ = Vec3f(1);
    Vec3f etaIn_  = Vec3f(1.39f);
    Vec3f k_ = Vec3f(3.9f);

public:

    using ResourceCommonImpl<IFresnel, Conductor>::ResourceCommonImpl;

    std::string Save() const override;

    void Load(const AGZ::ConfigGroup &params) override;

    std::string Export() const override;

    void Display() override;

    bool IsMultiline() const noexcept override;
};

DEFINE_DEFAULT_RESOURCE_CREATOR(IFresnelCreator, Conductor, "Conductor");

}; // namespace Atrc::Editor
