#include <Atrc/Light/LightSelector.h>

AGZ_NS_BEG(Atrc)

LightSelector::LightSelector(const std::vector<const Light*> &&lights)
    : lights_(lights)
{

}

LightSelectorSample LightSelector::Sample(const Intersection &inct) const
{
    if(lights_.empty())
        return { nullptr, 0.0 };
    int idx = Random::Uniform(0, static_cast<int>(lights_.size() - 1));
    return { lights_[idx], 1.0 / lights_.size() };
}

Real LightSelector::PDF(const Light *light) const
{
    return lights_.empty() ? 0.0 : (1.0 / lights_.size());
}

AGZ_NS_END(Atrc)
