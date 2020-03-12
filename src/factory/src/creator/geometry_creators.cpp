#include <agz/factory/creator/geometry_creators.h>
#include <agz/tracer/create/geometry.h>
#include <agz/tracer/utility/logger.h>

AGZ_TRACER_FACTORY_BEGIN

namespace geometry
{
    
    class DiskCreator : public Creator<Geometry>
    {
    public:

        std::string name() const override
        {
            return "disk";
        }

        RC<Geometry> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto local_to_world = params.child_transform3("transform");
            const real radius = params.child_real("radius");
            return create_disk(radius, local_to_world);
        }
    };

    class DoubleSidedGeometryCreator : public Creator<Geometry>
    {
    public:

        std::string name() const override
        {
            return "double_sided";
        }

        RC<Geometry> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto internal = context.create<Geometry>(params.child_group("internal"));
            return create_double_sided(std::move(internal));
        }
    };

    class QuadCreator : public Creator<Geometry>
    {
    public:

        std::string name() const override
        {
            return "quad";
        }

        RC<Geometry> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto local_to_world = params.child_transform3("transform");
            
            const Vec3 a = params.child_vec3("A");
            const Vec3 b = params.child_vec3("B");
            const Vec3 c = params.child_vec3("C");
            const Vec3 d = params.child_vec3("D");

            const Vec2 t_a = params.child_vec2_or("tA", Vec2(0));
            const Vec2 t_b = params.child_vec2_or("tB", Vec2(0));
            const Vec2 t_c = params.child_vec2_or("tC", Vec2(0));
            const Vec2 t_d = params.child_vec2_or("tD", Vec2(0));

            return create_quad(a, b, c, d, t_a, t_b, t_c, t_d, local_to_world);
        }
    };

    class SphereCreator : public Creator<Geometry>
    {
    public:

        std::string name() const override
        {
            return "sphere";
        }

        RC<Geometry> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto local_to_world = params.child_transform3("transform");
            const real radius = params.child_real("radius");
            return create_sphere(radius, local_to_world);
        }
    };

    class TransformWrapperCreator : public Creator<Geometry>
    {
    public:

        std::string name() const override
        {
            return "transform_wrapper";
        }

        RC<Geometry> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            auto internal = context.create<Geometry>(params.child_group("internal"));
            const auto local_to_world = params.child_transform3("transform");
            return create_transform_wrapper(std::move(internal), local_to_world);
        }
    };

    class TriangleCreator : public Creator<Geometry>
    {
    public:

        std::string name() const override
        {
            return "triangle";
        }

        RC<Geometry> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto local_to_world = params.child_transform3("transform");

            const Vec3 a = params.child_vec3("A");
            const Vec3 b = params.child_vec3("B");
            const Vec3 c = params.child_vec3("C");

            const Vec2 t_a = params.child_vec2_or("tA", Vec2(0));
            const Vec2 t_b = params.child_vec2_or("tB", Vec2(0));
            const Vec2 t_c = params.child_vec2_or("tC", Vec2(0));

            return create_triangle(a, b, c, t_a, t_b, t_c, local_to_world);
        }
    };

    class TriangleBVHNoEmbreeCreator : public Creator<Geometry>
    {
    public:

        std::string name() const override
        {
            return "triangle_bvh_noembree";
        }

        RC<Geometry> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto local_to_world = params.child_transform3("transform");
            const auto filename = context.path_mapper->map(params.child_str("filename"));

            AGZ_INFO("load mesh from {}", filename);
            auto build_triangles = mesh::load_from_file(filename);
            AGZ_INFO("triangle count: {}", build_triangles.size());

            return create_triangle_bvh_noembree(
                std::move(build_triangles), local_to_world);
        }
    };

#ifdef USE_EMBREE

    class TriangleBVHEmbreeCreator : public Creator<Geometry>
    {
    public:

        std::string name() const override
        {
            return "triangle_bvh_embree";
        }

        RC<Geometry> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto local_to_world = params.child_transform3("transform");
            const auto filename = context.path_mapper->map(params.child_str("filename"));

            AGZ_INFO("load mesh from {}", filename);
            auto build_triangles = mesh::load_from_file(filename);
            AGZ_INFO("triangle count: {}", build_triangles.size());

            return create_triangle_bvh_embree(std::move(build_triangles), local_to_world);
        }
    };

    class TriangleBVHCreator : public TriangleBVHEmbreeCreator
    {
    public:

        std::string name() const override
        {
            return "triangle_bvh";
        }
    };

#else

    class TriangleBVHCreator : public TriangleBVHNoEmbreeCreator
    {
    public:

        std::string name() const override
        {
            return "triangle_bvh";
        }
    };

#endif

} // namespace geometry

void initialize_geometry_factory(Factory<Geometry> &factory)
{
    factory.add_creator(newBox<geometry::DiskCreator>());
    factory.add_creator(newBox<geometry::DoubleSidedGeometryCreator>());
    factory.add_creator(newBox<geometry::QuadCreator>());
    factory.add_creator(newBox<geometry::SphereCreator>());
    factory.add_creator(newBox<geometry::TransformWrapperCreator>());
    factory.add_creator(newBox<geometry::TriangleCreator>());
    factory.add_creator(newBox<geometry::TriangleBVHCreator>());
    factory.add_creator(newBox<geometry::TriangleBVHNoEmbreeCreator>());
#ifdef USE_EMBREE
    factory.add_creator(newBox<geometry::TriangleBVHEmbreeCreator>());
#endif
}

AGZ_TRACER_FACTORY_END
