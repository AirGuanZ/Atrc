#include <agz/tracer/core/camera.h>

AGZ_TRACER_BEGIN

class ThinlensCamera : public Camera
{
    Vec3 pos_;
    Vec3 film_left_bottom_;
    Vec3 x_ori_;
    Vec3 y_ori_;
    Vec3 dir_;
    real dist_ = 1;

    real lens_radius_ = 1;
    Vec3 lens_u_;
    Vec3 lens_v_;
    real lens_area_ = PI_r;
    real lookat_dist_ = 20;
    Vec3 lens_centre_;

    real film_area_in_world_space_ = 1;

public:

    static std::string description()
    {
        return R"___(
thinlens [Camera]
    pos         [Vec3] eye position
    dst         [Vec3] lookat position
    up          [Vec3] aux vector; commonly used in perspective camera model
    width       [real] sensor width in world space
    height      [real] (optional) sensor height in world space
    aspect      [real] (required only when 'height' is not specified) width/height
    dist        [real] distance between film and lens
    lens_radius [real] thinlens radius
    lookat_dist [real] distance between lookat point and lens
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        Vec3 pos = params.child_vec3("pos");
        Vec3 dst = params.child_vec3("dst");
        Vec3 up  = params.child_vec3("up");

        real sensor_width = params.child_real("width");
        real sensor_height;
        if(auto sensor_height_node = params.find_child("height"))
            sensor_height = sensor_height_node->as_value().as_real();
        else
        {
            auto film_width_over_height = params.child_real("aspect");
            sensor_height = sensor_width / film_width_over_height;
        }

        real dist = params.child_real("dist");

        Vec3 dir = (dst - pos).normalize();

        Vec3 x_dir = cross(dir, up).normalize();
        Vec3 y_dir = cross(x_dir, dir).normalize();

        real x_len = sensor_width;
        real y_len = sensor_height;

        pos_   = pos;
        dir_   = dir;
        dist_  = dist;
        x_ori_ = x_len * x_dir;
        y_ori_ = y_len * y_dir;
        film_left_bottom_ = pos_ - real(0.5) * x_ori_ - real(0.5) * y_ori_;
        film_area_in_world_space_ = x_len * y_len;

        lens_radius_ = params.child_real("lens_radius");
        lens_u_ = lens_radius_ * x_dir;
        lens_v_ = lens_radius_ * y_dir;
        lens_area_ = PI_r * lens_radius_ * lens_radius_;
        lens_centre_ = pos_ + dist_ * dir_;

        lookat_dist_ = params.child_real("lookat_dist");

        AGZ_HIERARCHY_WRAP("in initializing thinlens camera")
    }

    AABB world_bound() const noexcept override
    {
        AABB ret;
        ret |= film_left_bottom_;
        ret |= film_left_bottom_ + x_ori_;
        ret |= film_left_bottom_ + y_ori_;
        ret |= film_left_bottom_ + x_ori_ + y_ori_;

        for(int i = 0; i < 3; ++i)
        {
            real dem = std::sqrt(lens_u_[i] * lens_u_[i] + lens_v_[i] * lens_v_[i]);
            real cos = lens_u_[i] / dem, sin = lens_v_[i] / dem;

            ret |= lens_centre_ + sin * lens_u_ + cos * lens_v_;
            ret |= lens_centre_ - sin * lens_u_ + cos * lens_v_;
            ret |= lens_centre_ + sin * lens_u_ - cos * lens_v_;
            ret |= lens_centre_ - sin * lens_u_ - cos * lens_v_;
        }

        return ret;
    }

    CameraGenerateRayResult generate_ray(const CameraSample &sam) const noexcept override
    {
        auto uniform_on_disk = math::distribution::uniform_on_unit_disk(sam.aperture_sample.u, sam.aperture_sample.v);
        Vec3 pos_on_sensor = film_left_bottom_ + sam.film_coord.u * x_ori_ + sam.film_coord.v * y_ori_;
        Vec3 pos_on_lens   = lens_centre_ + uniform_on_disk.x * lens_u_ + uniform_on_disk.y * lens_v_;

        // TODO

        return {};
    }
};

AGZ_TRACER_END
