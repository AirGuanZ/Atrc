#include <agz/tracer/factory/creator/camera_creators.h>
#include <agz/tracer/factory/raw/camera.h>

AGZ_TRACER_FACTORY_BEGIN

namespace camera
{

    /*
     * pinhole已被thin_lens camera替代，此处只是为了兼容过去的写法
     */
    class PinholeCameraCreator : public Creator<Camera>
    {
    public:

        std::string name() const override
        {
            return "pinhole";
        }

        std::shared_ptr<Camera> create(const ConfigGroup &params, CreatingContext &context, int film_width, int film_height) const override
        {
            const Vec3 pos = params.child_vec3("pos");
            const Vec3 dst = params.child_vec3("dst");
            const Vec3 up  = params.child_vec3("up");

            const real aspect = params.child_real("film_aspect");
            const real width  = params.child_real("width");
            const real height = width / aspect;
            const real dist   = params.child_real("dist");
            const real fov    = 2 * std::atan(height / (2 * dist));

            return create_thin_lens_camera(film_width, film_height, pos, dst, up, fov, aspect, 0, 1);
        }
    };

    class PerspectiveCameraCreator : public Creator<Camera>
    {
    public:

        std::string name() const override
        {
            return "thin_lens";
        }

        std::shared_ptr<Camera> create(const ConfigGroup &params, CreatingContext &context, int film_width, int film_height) const override
        {
            const Vec3 pos = params.child_vec3("pos");
            const Vec3 dst = params.child_vec3("dst");
            const Vec3 up  = params.child_vec3("up");

            const real fov    = math::deg2rad(params.child_real("fov"));
            const real aspect = params.child_real("film_aspect");

            const real lens_radius    = params.child_real_or("lens_radius", 0);
            const real focal_distance = params.child_real_or("focal_distance", 1);

            return create_thin_lens_camera(film_width, film_height, pos, dst, up, fov, aspect, lens_radius, focal_distance);
        }
    };

} // namespace camera

void initialize_camera_factory(Factory<Camera> &factory)
{
    factory.add_creator(std::make_unique<camera::PinholeCameraCreator>());
    factory.add_creator(std::make_unique<camera::PerspectiveCameraCreator>());
}

AGZ_TRACER_FACTORY_END
