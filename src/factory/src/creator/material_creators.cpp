#include <agz/factory/creator/material_creators.h>
#include <agz/factory/help.h>
#include <agz/tracer/create/material.h>

AGZ_TRACER_FACTORY_BEGIN

namespace material
{
    Box<NormalMapper> init_normal_mapper(
        const ConfigGroup &params, CreatingContext &context)
    {
        if(auto node = params.find_child_group("normal_map"))
        {
            const auto map = context.create<Texture2D>(*node);
            return newBox<NormalMapper>(map);
        }
        return newBox<NormalMapper>(nullptr);
    }

    RC<BSSRDFSurface> init_bssrdf_surface(
        const ConfigGroup &params,
        RC<const Texture2D> default_A,
        RC<const Texture2D> default_eta,
        CreatingContext &context)
    {
        if(!params.find_child("bssrdf_dmfp"))
            return newRC<BSSRDFSurface>();

        auto dmfp = context.create<Texture2D>(params.child_group("bssrdf_dmfp"));

        auto A = default_A;
        if(auto nodeA = params.find_child_group("bssrdf_A"))
            A = context.create<Texture2D>(*nodeA);
        if(!A)
            throw ObjectConstructionException("bssrdf_A not found");

        auto eta = default_eta;
        if(auto node_eta = params.find_child_group("bssrdf_eta"))
            eta = context.create<Texture2D>(*node_eta);
        if(!eta)
            throw ObjectConstructionException("bssrdf_eta not found");

        return create_normalized_diffusion_bssrdf_surface(
            std::move(A), std::move(dmfp), std::move(eta));
    }
    
    class DisneyCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "disney";
        }

        RC<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto base_color   = context.create<Texture2D>(
                params.child_group("base_color"));

            const auto metallic     = context.create<Texture2D>(
                params.child_group("metallic"));

            const auto roughness    = context.create<Texture2D>(
                params.child_group("roughness"));

            const auto transmission = helper::child_texture_or_constant(
                context, params, "transmission", 0);
            
            auto transmission_roughness = roughness;
            if(auto node = params.find_child_group("transmission_roughness"))
                transmission_roughness = context.create<Texture2D>(*node);
            
            const auto ior              = helper::child_texture_or_constant(
                context, params, "ior", real(1.5));
            const auto specular_scale   = helper::child_texture_or_constant(
                context, params, "specular_scale", 1);
            const auto specular_tint    = helper::child_texture_or_constant(
                context, params, "specular_tint", 0);
            const auto anisotropic      = helper::child_texture_or_constant(
                context, params, "anisotropic", 0);
            const auto sheen            = helper::child_texture_or_constant(
                context, params, "sheen", 0);
            const auto sheen_tint       = helper::child_texture_or_constant(
                context, params, "sheen_tint", 0);
            const auto clearcoat        = helper::child_texture_or_constant(
                context, params, "clearcoat", 0);
            const auto clearcoat_gloss  = helper::child_texture_or_constant(
                context, params, "clearcoat_gloss", 1);

            auto normal_mapper = init_normal_mapper(params, context);

            auto bssrdf = init_bssrdf_surface(params, base_color, ior, context);

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
                std::move(normal_mapper),
                std::move(bssrdf));
        }
    };

    class GlassCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "glass";
        }

        RC<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto ior = context.create<Texture2D>(params.child_group("ior"));

            RC<Texture2D> color_reflection_map, color_refraction_map;

            if(auto color_map_node = params.find_child_group("color_map"))
            {
                auto color_map = context.create<Texture2D>(*color_map_node);
                color_reflection_map = color_map;
                color_refraction_map = color_map;
            }

            if(auto node = params.find_child_group("color_reflection_map"))
                color_reflection_map = context.create<Texture2D>(*node);

            if(auto node = params.find_child_group("color_refraction_map"))
                color_refraction_map = context.create<Texture2D>(*node);

            if(!color_reflection_map || !color_refraction_map)
                throw CreatingObjectException(
                    "empty color reflection/refraction map");

            auto bssrdf = init_bssrdf_surface(params, {}, ior, context);

            return create_glass(
                std::move(color_reflection_map),
                std::move(color_refraction_map),
                std::move(ior),
                std::move(bssrdf));
        }
    };

    class IdealBlackCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "ideal_black";
        }

        RC<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
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

        RC<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto albedo = context.create<Texture2D>(
                params.child_group("albedo"));
            auto normal_mapper = init_normal_mapper(params, context);
            return create_ideal_diffuse(
                std::move(albedo), std::move(normal_mapper));
        }
    };

    class InvisibleSurfaceCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "invisible_surface";
        }

        RC<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            auto bssrdf = init_bssrdf_surface(params, {}, {}, context);
            return create_invisible_surface(std::move(bssrdf));
        }
    };

    class MetalCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "metal";
        }

        RC<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            auto texture2d = [&](const char *name)
            {
                return context.create<Texture2D>(params.child_group(name));
            };

            auto color       = texture2d("color");
            auto eta         = texture2d("eta");
            auto k           = texture2d("k");
            auto roughness   = texture2d("roughness");
            auto anisotropic = texture2d("anisotropci");

            auto normal_mapper = init_normal_mapper(params, context);

            return create_metal(
                std::move(color),
                std::move(eta),
                std::move(k),
                std::move(roughness),
                std::move(anisotropic),
                std::move(normal_mapper));
        }
    };

    class MirrorCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "mirror";
        }

        RC<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            auto color_map = context.create<Texture2D>(
                params.child_group("color_map"));
            auto eta = context.create<Texture2D>(params.child_group("eta"));
            auto k   = context.create<Texture2D>(params.child_group("k"));
            return create_mirror(
                std::move(color_map), std::move(eta), std::move(k));
        }
    };

    class PhongCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "phong";
        }

        RC<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            auto d  = context.create<Texture2D>(params.child_group("d"));
            auto s  = context.create<Texture2D>(params.child_group("s"));
            auto ns = context.create<Texture2D>(params.child_group("ns"));
            auto nor_map = init_normal_mapper(params, context);
            return create_phong(
                std::move(d), std::move(s), std::move(ns), std::move(nor_map));
        }
    };

    class SSSWrapperCreator : public Creator<Material>
    {
    public:

        std::string name() const override
        {
            return "sss";
        }

        std::shared_ptr<Material> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            auto A    = context.create<Texture2D>(params.child_group("A"));
            auto dmfp = context.create<Texture2D>(params.child_group("dmfp"));
            auto eta  = context.create<Texture2D>(params.child_group("eta"));

            auto internal = context.create<Material>(
                params.child_group("internal"));

            return create_sss_wrapper(
                std::move(A), std::move(dmfp),
                std::move(eta), std::move(internal));
        }
    };

} // namespace material;

void initialize_material_factory(Factory<Material> &factory)
{
    factory.add_creator(newBox<material::DisneyCreator>());
    factory.add_creator(newBox<material::GlassCreator>());
    factory.add_creator(newBox<material::IdealBlackCreator>());
    factory.add_creator(newBox<material::IdealDiffuseCreator>());
    factory.add_creator(newBox<material::InvisibleSurfaceCreator>());
    factory.add_creator(newBox<material::MetalCreator>());
    factory.add_creator(newBox<material::MirrorCreator>());
    factory.add_creator(newBox<material::PhongCreator>());
    factory.add_creator(newBox<material::SSSWrapperCreator>());
}

AGZ_TRACER_FACTORY_END
