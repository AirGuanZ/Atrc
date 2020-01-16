#include <agz/tracer/factory/creator/envir_light_creators.h>
#include <agz/tracer/factory/raw/envir_light.h>

AGZ_TRACER_FACTORY_BEGIN

namespace envir_light
{

    class IBLEnvirLightCreator : public Creator<EnvirLight>
    {
    public:

        std::string name() const override
        {
            return "ibl";
        }

        std::shared_ptr<EnvirLight> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto tex = context.create<Texture2D>(params.child_group("tex"));
            const auto up = params.child_vec3_or("up", Vec3(0, 0, 1));
            return create_ibl_light(std::move(tex), up);
        }
    };

    class NativeSkyCreator : public Creator<EnvirLight>
    {
    public:

        std::string name() const override
        {
            return "native_sky";
        }

        std::shared_ptr<EnvirLight> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto top = params.child_spectrum("top");
            const auto bottom = params.child_spectrum("bottom");
            const auto up = params.child_vec3_or("up", Vec3(0, 0, 1));
            return create_native_sky(top, bottom, up);
        }
    };

} // namespace envir_light

void initialize_envir_light_factory(Factory<EnvirLight> &factory)
{
    factory.add_creator(std::make_unique<envir_light::IBLEnvirLightCreator>());
    factory.add_creator(std::make_unique<envir_light::NativeSkyCreator>());
}

AGZ_TRACER_FACTORY_END
