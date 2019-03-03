#pragma once

#include <Atrc/Lib/Core/ResourceData.h>
#include <Atrc/Lib/Core/TFilm.h>

namespace Atrc
{

class GaussianFilterData : public ResourceData<FilmFilter>
{
    Real radius_ = 1;
    Real alpha_ = 2;

public:

    static const std::string &GetTypeName() { static const std::string RET = "Gaussian"; return RET; }

    std::string Serialize() const override;

    void Deserialize(const AGZ::ConfigGroup &param) override;

    FilmFilter *CreateResource(Arena &arena) const override;
};

} // namespace Atrc
