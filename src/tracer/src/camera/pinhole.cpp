#include <agz/tracer/core/camera.h>

AGZ_TRACER_BEGIN

class PinholeCamera : public Camera
{
    Vec3 pos_;
    Vec3 film_left_bottom_;
    Vec3 x_ori_;
    Vec3 y_ori_;
    Vec3 dir_;
    real dist_ = 1;

    real film_area_in_world_space_ = 1;

public:

    using Camera::Camera;

    static std::string description()
    {
        return R"___(
pinhole [Camera]
    pos    [Vec3] eye position
    dst    [Vec3] lookat position
    up     [Vec3] aux vector; commonly used in perspective camera model
    width  [real] sensor width in world space
    height [real] (optional) sensor height in world space
    aspect [real] (required only when 'height' is not specified) width/height
    dist   [real] distance between eye point and sensor plane
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        Vec3 pos = params.child_vec3("pos");
        Vec3 dst = params.child_vec3("dst");
        Vec3 up = params.child_vec3("up").normalize();

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
        Vec3 film_centre = pos + dist * dir;

        Vec3 x_dir = cross(dir, up).normalize();
        Vec3 y_dir = cross(x_dir, dir).normalize();

        real x_len = sensor_width;
        real y_len = sensor_height;

        pos_ = pos;
        dir_ = dir;
        dist_ = dist;
        x_ori_ = x_len * x_dir;
        y_ori_ = y_len * y_dir;
        film_left_bottom_ = film_centre - real(0.5) * x_ori_ - real(0.5) * y_ori_;

        film_area_in_world_space_ = x_len * y_len;

        AGZ_HIERARCHY_WRAP("in initializing pinhole camera")
    }

    AABB world_bound() const noexcept override
    {
        AABB ret;
        ret |= pos_;
        ret |= film_left_bottom_;
        ret |= film_left_bottom_ + x_ori_;
        ret |= film_left_bottom_ + y_ori_;
        ret |= film_left_bottom_ + x_ori_ + y_ori_;
        return ret;
    }

    CameraGenerateRayResult generate_ray(const CameraSample &sam) const noexcept override
    {
        CameraGenerateRayResult ret;

        Vec3 dst = film_left_bottom_ + sam.film_coord.u * x_ori_ + sam.film_coord.v * y_ori_;
        Vec3 dir = (dst - pos_).normalize();

        real cos_v = cos(dir, dir_);

        ret.r            = Ray(pos_, dir);
        ret.pdf_pos      = 1 / film_area_in_world_space_;
        ret.is_pos_delta = false;
        ret.pdf_dir      = dist_ * dist_ / (cos_v * cos_v * cos_v);
        ret.is_dir_delta = true;
        ret.importance   = dist_ * dist_ / (film_area_in_world_space_ * cos_v * cos_v * cos_v * cos_v);

        return ret;
    }

    CameraSampleRayResult sample(const Vec3 &ref_pos, const Sample2 &) const noexcept override
    {
        Vec3 dref = (ref_pos - pos_).normalize();
        real proj_dref = dot(dref, dir_);
        if(proj_dref <= EPS)
            return CAMERA_SAMPLE_RAY_RESULT_INVALID;
        Vec3 pos_on_sensor = pos_ + dist_ / proj_dref * dref;
        Vec3 offset = pos_on_sensor - film_left_bottom_;

        real film_u = dot(offset, x_ori_) / x_ori_.length_square();
        real film_v = dot(offset, y_ori_) / y_ori_.length_square();

        real cos_v = cos(dref, dir_);

        CameraSampleRayResult ret;
        ret.ref_to_cam = -dref;
        ret.film_pos   = pos_;
        ret.film_coord = { film_u, film_v };
        ret.importance = dist_ * dist_ / (film_area_in_world_space_ * cos_v * cos_v * cos_v * cos_v);
        ret.pdf        = (ref_pos - pos_).length_square() / cos(dref, dir_);
        ret.is_delta   = true;

        return ret;
    }
};

AGZT_IMPLEMENTATION(Camera, PinholeCamera, "pinhole")

AGZ_TRACER_END
