#pragma once

#include <Atrc/Common.h>
#include <Atrc/Light/Light.h>

AGZ_NS_BEG(Atrc)

struct GeometrySurfaceSample
{
    Vec3r pos;
    Vec3r nor;
    Real pdf = 1.0;
};

template<typename GeoTpl, std::enable_if_t<std::is_base_of_v<Entity, GeoTpl>, int> = 0,
                          typename = AGZ::TypeOpr::Void_t<decltype(&GeoTpl::SampleSurface)>>
class GeoDiffuseLight
    : ATRC_IMPLEMENTS Light,
      ATRC_PROPERTY AGZ::Uncopiable,
      public GeoTpl
{
    Spectrum color_;

public:

    template<typename...Args>
    explicit GeoDiffuseLight(const Spectrum &color, Args&&...args)
        : GeoTpl(std::forward<Args>(args)...), color_(color)
    {
        
    }

    Option<LightSample> SampleTo(const Intersection &inct) const override
    {
        auto geoSam = GeoTpl::SampleSurface();
        if(!geoSam)
            return None;

        LightSample ret;
        ret.pos      = geoSam->pos;
        ret.nor      = geoSam->nor;
        ret.radiance = color_;
        ret.pdf      = geoSam->pdf;

        return ret;
    }

    Spectrum Le(const Intersection &inct) const override
    {
        AGZ_ASSERT(dynamic_cast<const GeoDiffuseLight<GeoTpl>*>(inct.entity) == this);
        return color_;
    }

    const Light *AsLight() const override
    {
        return static_cast<const Light*>(this);
    }
};

AGZ_NS_END(Atrc)
