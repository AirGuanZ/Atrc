#include <AGZUtils/String/StdStr.h>
#include <Atrc/Lib/FilmFilter/BoxFilter.h>
#include <Atrc/Lib/Utility/ConfigConvert.h>

namespace Atrc
{

namespace
{
    class BoxFilter : public FilmFilter
    {
    public:

        explicit BoxFilter(const Vec2 &radius) noexcept
            : FilmFilter(radius)
        {

        }

        Real Eval(Real relX, Real relY) const noexcept override
        {
            return 1;
        }
    };
} // namespace anonymous

std::string BoxFilterData::Serialize() const
{
    static const AGZ::TFormatter<char> fmt(
        "type = Box;"
        "sidelen = {};");
    return "{" + fmt.Arg(sidelen_) + "}";
}

void BoxFilterData::Deserialize(const AGZ::ConfigGroup &param)
{
    AGZ_ASSERT(param["type"].AsValue() == GetTypeName());
    sidelen_ = param["sidelen"].Parse<Real>();
    if(sidelen_ <= 0)
        throw ResourceDataException("invalid sidelen value: " + std::to_string(sidelen_));
}

FilmFilter *BoxFilterData::CreateResource(Arena &arena) const
{
    return arena.Create<BoxFilter>(Vec2(sidelen_ / 2));
}

} // namespace Atrc
