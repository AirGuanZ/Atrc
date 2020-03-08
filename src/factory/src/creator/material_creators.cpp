#include <agz/factory/creator/material_creators.h>
#include <agz/factory/help.h>
#include <agz/tracer/create/material.h>

AGZ_TRACER_FACTORY_BEGIN

namespace material
{
    std::unique_ptr<NormalMapper> init_normal_mapper(const ConfigGroup &params, CreatingContext &context)
    {
        if(auto node = params.find_child_group("normal_map"))
        {
            const auto map = context.create<Texture2D>(*node);
            return std::make_unique<NormalMapper>(map);
        }
        return std::make_unique<NormalMapper>(nullptr);
    }
    
    class DisneyCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "disney";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto base_color   = context.create<Texture2D>(params.child_group("base_color"));
            const auto metallic     = context.create<Texture2D>(params.child_group("metallic"));
            const auto roughness    = context.create<Texture2D>(params.child_group("roughness"));
            const auto transmission = helper::child_texture_or_constant(context, params, "transmission", 0);
            
            auto transmission_roughness = roughness;
            if(auto node = params.find_child_group("transmission_roughness"))
                transmission_roughness = context.create<Texture2D>(*node);
            
            const auto ior              = helper::child_texture_or_constant(context, params, "ior", real(1.5));
            const auto specular_scale   = helper::child_texture_or_constant(context, params, "specular_scale", 1);
            const auto specular_tint    = helper::child_texture_or_constant(context, params, "specular_tint", 0);
            const auto anisotropic      = helper::child_texture_or_constant(context, params, "anisotropic", 0);
            const auto sheen            = helper::child_texture_or_constant(context, params, "sheen", 0);
            const auto sheen_tint       = helper::child_texture_or_constant(context, params, "sheen_tint", 0);
            const auto clearcoat        = helper::child_texture_or_constant(context, params, "clearcoat", 0);
            const auto clearcoat_gloss  = helper::child_texture_or_constant(context, params, "clearcoat_gloss", 1);

            auto normal_mapper = init_normal_mapper(params, context);

            return create_disney(
                std::move(base_color),
                std::move(metallic),
                std::move(roughness),
                std::move(transmission),
                std::move(transmission_roughness),
                std::move(ior),
                std::move(specular_scale),
                std::move(specular_tint),
                std::move(anisotropic),
                std::move(sheen),
                std::move(sheen_tint),
                std::move(clearcoat),
                std::move(clearcoat_gloss),
                std::move(normal_mapper));
        }
    };

    class DisneyReflectionCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "disney_reflection";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto base_color      = context.create<Texture2D>(params.child_group("base_color"));
            const auto metallic        = context.create<Texture2D>(params.child_group("metallic"));
            const auto roughness       = context.create<Texture2D>(params.child_group("roughness"));
            const auto subsurface      = helper::child_texture_or_constant(context, params, "subsurface", 0);
            const auto specular        = helper::child_texture_or_constant(context, params, "specular", 0);
            const auto specular_tint   = helper::child_texture_or_constant(context, params, "specular_tint", 0);
            const auto anisotropic     = helper::child_texture_or_constant(context, params, "anisotropic", 0);
            const auto sheen           = helper::child_texture_or_constant(context, params, "sheen", 0);
            const auto sheen_tint      = helper::child_texture_or_constant(context, params, "sheen_tint", 0);
            const auto clearcoat       = helper::child_texture_or_constant(context, params, "clearcoat", 0);
            const auto clearcoat_gloss = helper::child_texture_or_constant(context, params, "clearcoat_gloss", 1);

            auto normal_mapper = init_normal_mapper(params, context);

            return create_disney_reflection(
                std::move(base_color),
                std::move(metallic),
                std::move(roughness),
                std::move(subsurface),
                std::move(specular),
                std::move(specular_tint),
                std::move(anisotropic),
                std::move(sheen),
                std::move(sheen_tint),
                std::move(clearcoat),
                std::move(clearcoat_gloss),
                std::move(normal_mapper));
        }
    };

    class FrostedGlassCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "frosted_glass";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto color_map = context.create<Texture2D>(params.child_group("color_map"));
            const auto ior       = context.create<Texture2D>(params.child_group("ior"));
            const auto roughness = context.create<Texture2D>(params.child_group("roughness"));
            return create_frosted_glass(std::move(color_map), std::move(roughness), std::move(ior));
        }
    };

    class GlassCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "glass";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto ior = context.create<Texture2D>(params.child_group("ior"));

            std::shared_ptr<Texture2D> color_reflection_map, color_refraction_map;

            if(auto color_map_node = params.find_child_group("color_map"))
            {
                auto color_map = context.create<Texture2D>(*color_map_node);
                color_reflection_map = color_map;
                color_refraction_map = color_map;
            }

            if(auto color_reflection_map_node = params.find_child_group("color_reflection_map"))
                color_reflection_map = context.create<Texture2D>(*color_reflection_map_node);

            if(auto color_refraction_map_node = params.find_child_group("color_refraction_map"))
                color_refraction_map = context.create<Texture2D>(*color_refraction_map_node);

            if(!color_reflection_map || !color_refraction_map)
                throw CreatingObjectException("empty color reflection/refraction map");

            return create_glass(
                std::move(color_reflection_map),
                std::move(color_refraction_map),
                std::move(ior));
        }
    };

    class IdealBlackCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "ideal_black";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_ideal_black();
        }
    };

    class IdealDiffuseCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "ideal_diffuse";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto albedo = context.create<Texture2D>(params.child_group("albedo"));
            auto normal_mapper = init_normal_mapper(params, context);
            return create_ideal_diffuse(std::move(albedo), std::move(normal_mapper));
        }
    };

    class InvisibleSurfaceCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "invisible_surface";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            return create_invisible_surface();
        }
    };

    class MaterialAdderCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "add";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            std::vector<std::shared_ptr<const Material>> mats;
            
            auto &arr = params.child_array("mats");
            for(size_t i = 0; i < arr.size(); ++i)
                mats.push_back(context.create<Material>(arr.at(i).as_group()));

            return create_mat_adder(std::move(mats));
        }
    };

    class MaterialScalerCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "scale";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto internal = context.create<Material>(params.child_group("internal"));
            auto scale = context.create<Texture2D>(params.child_group("texture"));
            return create_mat_scaler(std::move(internal), std::move(scale));
        }
    };

    class MirrorCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "mirror";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto color_map = context.create<Texture2D>(params.child_group("color_map"));
            auto eta = context.create<Texture2D>(params.child_group("eta"));
            auto k   = context.create<Texture2D>(params.child_group("k"));
            return create_mirror(std::move(color_map), std::move(eta), std::move(k));
        }
    };

    class MTLCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "mtl";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto kd = context.create<Texture2D>(params.child_group("kd"));
            auto ks = context.create<Texture2D>(params.child_group("ks"));
            auto ns = context.create<Texture2D>(params.child_group("ns"));
            return create_mtl(std::move(kd), std::move(ks), std::move(ns));
        }
    };

    class MirrorVarnishCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "mirror_varnish";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto internal = context.create<Material>(params.child_group("internal"));
            auto color_map = context.create<Texture2D>(params.child_group("color_map"));
            auto eta_in = context.create<Texture2D>(params.child_group("eta_in"));
            auto eta_out = helper::child_texture_or_constant(context, params, "eta_out", 1);
            return create_mirror_varnish(std::move(internal), std::move(eta_in), std::move(eta_out), std::move(color_map));
        }
    };

    class PhongCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "phong";
        }

        std::shared_ptr<Material> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            auto d  = context.create<Texture2D>(params.child_group("d"));
            auto s  = context.create<Texture2D>(params.child_group("s"));
            auto ns = context.create<Texture2D>(params.child_group("ns"));
            auto nor_map = init_normal_mapper(params, context);
            return create_phong(std::move(d), std::move(s), std::move(ns), std::move(nor_map));
        }
    };

} // namespace material;

void initialize_material_factory(Factory<Material> &factory)
{
    factory.add_creator(std::make_unique<material::DisneyCreator>());
    factory.add_creator(std::make_unique<material::DisneyReflectionCreator>());
    factory.add_creator(std::make_unique<material::FrostedGlassCreator>());
    factory.add_creator(std::make_unique<material::GlassCreator>());
    factory.add_creator(std::make_unique<material::IdealBlackCreator>());
    factory.add_creator(std::make_unique<material::IdealDiffuseCreator>());
    factory.add_creator(std::make_unique<material::InvisibleSurfaceCreator>());
    factory.add_creator(std::make_unique<material::MaterialAdderCreator>());
    factory.add_creator(std::make_unique<material::MaterialScalerCreator>());
    factory.add_creator(std::make_unique<material::MirrorCreator>());
    factory.add_creator(std::make_unique<material::MTLCreator>());
    factory.add_creator(std::make_unique<material::MirrorVarnishCreator>());
    factory.add_creator(std::make_unique<material::PhongCreator>());
}

AGZ_TRACER_FACTORY_END
