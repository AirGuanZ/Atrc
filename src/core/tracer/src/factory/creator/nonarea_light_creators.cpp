#include <agz/tracer/factory/creator/nonarea_light_creators.h>
#include <agz/tracer/factory/raw/nonarea_light.h>

AGZ_TRACER_FACTORY_BEGIN

namespace envir_light
{
    
    class DirectionalEnvirLightCreator : public Creator<NonareaLight>
    {
    public:

        std::string name() const override
        {
            return "dir";
        }

        std::shared_ptr<NonareaLight> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto dir = params.child_vec3("dir");
            auto rad = params.child_spectrum("radiance");
            real range = params.child_real("range");
            return create_directional_light(dir, rad, range);
        }
    };

    class HDRIEnvirLightCreator : public Creator<NonareaLight>
    {
    public:

        std::string name() const override
        {
            return "hdri";
        }

        std::shared_ptr<NonareaLight> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto tex = context.create<Texture>(params.child_group("tex"));
            auto up = params.child_vec3_or("up", Vec3(0, 0, 1));
            auto radius = params.child_real_or("radius", 100);
            auto offset = params.child_vec3_or("offset", Vec3(0));
            return create_hdri_light(std::move(tex), up, radius, offset);
        }
    };

    class IBLEnvirLightCreator : public Creator<NonareaLight>
    {
    public:

        std::string name() const override
        {
            return "ibl";
        }

        std::shared_ptr<NonareaLight> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto tex = context.create<Texture>(params.child_group("tex"));
            auto up = params.child_vec3_or("up", Vec3(0, 0, 1));
            return create_ibl_light(std::move(tex), up);
        }
    };

    class NativeSkyCreator : public Creator<NonareaLight>
    {
    public:

        std::string name() const override
        {
            return "native_sky";
        }

        std::shared_ptr<NonareaLight> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto top = params.child_spectrum("top");
            auto bottom = params.child_spectrum("bottom");
            auto up = params.child_vec3_or("up", Vec3(0, 0, 1));
            return create_native_sky(top, bottom, up);
        }
    };

} // namespace envir_light

void initialize_nonarea_light_factory(Factory<NonareaLight> &factory)
{
    factory.add_creator(std::make_unique<envir_light::DirectionalEnvirLightCreator>());
    factory.add_creator(std::make_unique<envir_light::HDRIEnvirLightCreator>());
    factory.add_creator(std::make_unique<envir_light::IBLEnvirLightCreator>());
    factory.add_creator(std::make_unique<envir_light::NativeSkyCreator>());
}

AGZ_TRACER_FACTORY_END
