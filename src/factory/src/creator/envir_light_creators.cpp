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
            const bool no_importance_sampling = params.child_int_or(
                "no_importance_sampling", 0) != 0;
            const real power = params.child_real_or("power", -1);
            return create_ibl_light(
                std::move(tex), no_importance_sampling, power);
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
            const real power = params.child_real_or("power", -1);
            return create_native_sky(top, bottom, power);
        }
    };

} // namespace envir_light

void initialize_envir_light_factory(Factory<EnvirLight> &factory)
{
    factory.add_creator(newBox<envir_light::IBLEnvirLightCreator>());
    factory.add_creator(newBox<envir_light::NativeSkyCreator>());
}

AGZ_TRACER_FACTORY_END
