#include <agz/tracer/core/camera.h>
#include <agz/utility/misc.h>

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

    void initialize(
        const Vec3 &pos, const Vec3 &dst, const Vec3 &up,
        real sensor_width, real sensor_height, real dist)
    {
        AGZ_HIERARCHY_TRY

        if(sensor_width <= 0 || sensor_height <= 0)
            throw ObjectConstructionException("invalid sensor size value");
        
        if(dist <= 0)
            throw ObjectConstructionException("invalid dist value");

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
        Vec3 dst = film_left_bottom_ + sam.film_coord.u * x_ori_ + sam.film_coord.v * y_ori_;
        Vec3 dir = (dst - pos_).normalize();

        CameraGenerateRayResult ret;
        ret.r          = Ray(pos_, dir);
        ret.throughput = Spectrum(1);
        return ret;
    }
};

std::shared_ptr<Camera> create_pinhole(
    const Vec3 &pos,
    const Vec3 &dst,
    const Vec3 &up,
    real sensor_width,
    real sensor_aspect,
    real dist)
{
    auto ret = std::make_shared<PinholeCamera>();
    ret->initialize(pos, dst, up, sensor_width, sensor_width / sensor_aspect, dist);
    return ret;
}

AGZ_TRACER_END
