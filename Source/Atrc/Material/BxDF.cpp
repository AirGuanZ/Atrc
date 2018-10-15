#include <Atrc/Common.h>
#include <Atrc/Material/BxDF.h>

AGZ_NS_BEG(Atrc)

Spectrum BxDF::AmbientRadiance(const Intersection &inct) const
{
    return SPECTRUM::BLACK;
}

bool BxDF::CanScatter() const
{
    return true;
}

bool BxDF::IsSpecular() const
{
    return false;
}

const BxDF *BxDF::GetLeafBxDF() const
{
    return this;
}

AGZ_NS_END(Atrc)
