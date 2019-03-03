#include <Atrc/Lib/FilmFilter/GaussianFilter.h>

namespace Atrc
{
    
namespace
{
    Real Gaussian(Real d, Real expv, Real alpha)
    {
        return Max(Real(0), Real(Exp(-alpha * d * d) - expv));
    }

    class GaussianFilter : public FilmFilter
    {
        Real alpha_;
        Vec2 exp_;

    public:

        GaussianFilter(const Vec2 &radius, Real alpha) noexcept
            : FilmFilter(radius), alpha_(alpha),
            exp_(radius.Map([=](Real c) { return Exp(-alpha * c * c); }))
        {

        }

        Real Eval(Real relX, Real relY) const noexcept override
        {
            return Gaussian(relX, exp_.x, alpha_) * Gaussian(relY, exp_.y, alpha_);
        }
    };
} // namespace anonymous

namespace
{
}

std::string GaussianFilterData::Serialize() const
{
    static const AGZ::TFormatter<char> fmt(
        "type = Gaussian;"
        "radius = {};"
        "alpha = {};");
    return "{" + fmt.Arg(radius_, alpha_) + "}";
}

void GaussianFilterData::Deserialize(const AGZ::ConfigGroup &param)
{
    AGZ_ASSERT(param["type"].AsValue() == "Gaussian");
    radius_ = param["radius"].Parse<Real>();
    alpha_ = param["alpha"].Parse<Real>();

    if(radius_ <= 0)
        throw ResourceDataException("invalid radius value: " + std::to_string(radius_));
    if(alpha_ <= 0)
        throw ResourceDataException("invalid alpha value: " + std::to_string(alpha_));
}

FilmFilter *GaussianFilterData::CreateResource(Arena &arena) const
{
    return arena.Create<GaussianFilter>(Vec2(radius_), alpha_);
}

} // namespace Atrc
