#include <agz/factory/creator/envir_light_creators.h>
#include <agz/tracer/create/envir_light.h>

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

        RC<EnvirLight> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto tex = context.create<Texture2D>(
                params.child_group("tex"));
            return create_ibl_light(std::move(tex));
        }
    };

    class NativeSkyCreator : public Creator<EnvirLight>
    {
    public:

        std::string name() const override
        {
            return "native_sky";
        }

        RC<EnvirLight> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto top    = params.child_spectrum("top");
            const auto bottom = params.child_spectrum("bottom");
            return create_native_sky(top, bottom);
        }
    };

} // namespace envir_light

void initialize_envir_light_factory(Factory<EnvirLight> &factory)
{
    factory.add_creator(newBox<envir_light::IBLEnvirLightCreator>());
    factory.add_creator(newBox<envir_light::NativeSkyCreator>());
}

AGZ_TRACER_FACTORY_END
