#include <agz/tracer/factory/creator/camera_creators.h>
#include <agz/tracer/factory/raw/camera.h>

AGZ_TRACER_FACTORY_BEGIN

namespace camera
{
    
    class PinholeCameraCreator : public Creator<Camera>
    {
    public:

        std::string name() const override
        {
            return "pinhole";
        }

        std::shared_ptr<Camera> create(const ConfigGroup &params, CreatingContext &context) const override
        {
            Vec3 pos = params.child_vec3("pos");
            Vec3 dst = params.child_vec3("dst");
            Vec3 up = params.child_vec3("up");

            real width = params.child_real("width");
            real aspect;
            if(auto node = params.find_child_value("height"))
                aspect = width / node->as_real();
            else
                aspect = params.child_real("aspect");
            real dist = params.child_real("dist");

            return create_pinhole(pos, dst, up, width, aspect, dist);
        }
    };

} // namespace camera

void initialize_camera_factory(Factory<Camera> &factory)
{
    factory.add_creator(std::make_unique<camera::PinholeCameraCreator>());
}

AGZ_TRACER_FACTORY_END
