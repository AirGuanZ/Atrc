#include <agz/tracer/core/camera.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class ThinLensCamera : public Camera
{
    Vec3 pos_;
    Vec3 dir_;

    real focal_film_width_  = 1;
    real focal_film_height_ = 1;

    real area_focal_film_ = 1;
    real area_lens_       = 1;

    real lens_radius_ = 0;
    real focal_distance_ = 1;

    Transform3 camera_to_world_;

    struct Params
    {
        int film_width  = 0;
        int film_height = 0;
        Vec3 pos;
        Vec3 dst;
        Vec3 up;
        real fov            = 0;
        real lens_radius    = 0;
        real focal_distance = 0;
    } params_;

    void init_from_params(const Params &params)
    {
        const real aspect = static_cast<real>(params.film_width) / params.film_height;

        focal_film_height_ = 2 * params.focal_distance * std::tan(params.fov / 2);
        focal_film_width_ = aspect * focal_film_height_;

        area_focal_film_ = focal_film_width_ * focal_film_height_;
        area_lens_ = params.lens_radius > 0 ? PI_r * params.lens_radius * params.lens_radius : 1;

        camera_to_world_ = Transform3(Trans4::look_at(params.pos, params.dst, params.up)).inv();

        pos_ = params.pos;
        dir_ = (params.dst - params.pos).normalize();

        lens_radius_ = params.lens_radius;
        focal_distance_ = params.focal_distance;
    }

public:

    ThinLensCamera(
        int film_width, int film_height,
        const Vec3 &pos, const Vec3 &dst, const Vec3 &up,
        real fov, real lens_radius, real focal_distance)
        : Camera(film_width, film_height)
    {
        params_.film_width     = film_width;
        params_.film_height    = film_height;
        params_.pos            = pos;
        params_.dst            = dst;
        params_.up             = up;
        params_.fov            = fov;
        params_.lens_radius    = lens_radius;
        params_.focal_distance = focal_distance;
        init_from_params(params_);
    }

    void update_param(std::string_view name, const std::any &value) override
    {
        if(name == "film_width")
            params_.film_width = std::any_cast<int>(value);
        else if(name == "film_height")
            params_.film_height = std::any_cast<int>(value);
        else
            throw ObjectConstructionException("unnown updated param: " + std::string(name));
        init_from_params(params_);
    }

    CameraSampleWeResult sample_we(const Vec2 &film_coord, const Sample2 &aperture_sample) const noexcept override
    {
        const Vec3 focal_film_pos = {
            (real(0.5) - film_coord.x) * focal_film_width_,
            (film_coord.y - real(0.5)) * focal_film_height_,
            focal_distance_
        };
        const Vec2 disk_sam = math::distribution::uniform_on_unit_disk(aperture_sample.u, aperture_sample.v);
        const Vec3 lens_pos = Vec3(lens_radius_ * disk_sam.x, lens_radius_ * disk_sam.y, 0);

        CameraSampleWeResult ret;
        ret.pos_on_cam = camera_to_world_.apply_to_point(lens_pos);
        ret.pos_to_out = camera_to_world_.apply_to_vector(focal_film_pos - lens_pos).normalize();
        ret.nor_on_cam = camera_to_world_.apply_to_vector(Vec3(0, 0, 1)).normalize();
        ret.throughput = Spectrum(1);

        return ret;
    }

    CameraEvalWeResult eval_we(const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept override
    {
        // 若方向和摄像机朝向差距超过180度，丢弃

        const Vec3 lens_pos  = camera_to_world_.apply_inverse_to_point(pos_on_cam);
        const Vec3 local_dir = camera_to_world_.apply_inverse_to_vector(pos_to_out).normalize();
        if(local_dir.z <= 0)
            return CAMERA_EVAL_WE_RESULT_ZERO;

        // 计算film coord

        const Vec3 focal_film_pos = lens_pos + (focal_distance_ / local_dir.z) * local_dir;

        // 丢弃film coord超出采样范围的结果

        const Vec2 film_coord = {
            real(0.5) - focal_film_pos.x / focal_film_width_,
            real(0.5) + focal_film_pos.y / focal_film_height_
        };
        const real cos_theta  = local_dir.z;
        const real cos2_theta = cos_theta * cos_theta;
        const real we         = focal_distance_ * focal_distance_ / (area_focal_film_ * area_lens_ * cos2_theta * cos2_theta);

        return { Spectrum(we), film_coord };
    }

    CameraWePDFResult pdf_we(const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept override
    {
        // 若方向和摄像机朝向差距超过180度，则方向pdf为0

        const Vec3 local_dir = camera_to_world_.apply_inverse_to_vector(pos_to_out).normalize();
        if(local_dir.z <= 0)
            return { 1 / area_lens_, 0 };

        // 若film coord超出采样范围，则方向pdf为0

        const real cos_theta = local_dir.z;
        const real pdf_dir   = focal_distance_ * focal_distance_ / (area_focal_film_ * cos_theta * cos_theta * cos_theta);

        return { 1 / area_lens_, pdf_dir };
    }

    CameraSampleWiResult sample_wi(const Vec3 &ref, const Sample2 &sam) const noexcept override
    {
        const Vec3 local_ref = camera_to_world_.apply_inverse_to_point(ref);

        // 采样镜头

        const Vec2 disk_sam = math::distribution::uniform_on_unit_disk(sam.u, sam.v);
        const Vec2 lens_sam = lens_radius_ * disk_sam;
        const Vec3 lens_pos = { lens_sam.x, lens_sam.y, 0 };

        // 计算方向

        const Vec3 local_dir = (local_ref - lens_pos).normalize();
        if(local_dir.z <= 0)
            return CAMERA_SAMPLE_WI_RESULT_INVALID;

        // 计算focal film上的位置

        const Vec3 focal_film_pos = lens_pos + (focal_distance_ / local_dir.z) * local_dir;
        const Vec2 film_coord = {
            real(0.5) - focal_film_pos.x / focal_film_width_,
            real(0.5) + focal_film_pos.y / focal_film_height_
        };

        CameraSampleWiResult ret;
        ret.pos_on_cam = camera_to_world_.apply_to_point(lens_pos);
        ret.nor_at_pos = dir_;
        ret.ref_to_pos = ret.pos_on_cam - ref;
        ret.we         = this->eval_we(ret.pos_on_cam, -ret.ref_to_pos).we;
        ret.pdf        = (ret.pos_on_cam - ref).length_square() / (local_dir.z * area_lens_);
        ret.film_coord = film_coord;

        return ret;
    }

    AABB get_world_bound() const noexcept override
    {
        return { pos_ - Vec3(real(0.1)), pos_ + Vec3(real(0.1)) };
    }
};

std::shared_ptr<Camera> create_thin_lens_camera(
    int film_width, int film_height,
    const Vec3 &pos,
    const Vec3 &dst,
    const Vec3 &up,
    real fov,
    real lens_radius,
    real focal_distance)
{
    return std::make_shared<ThinLensCamera>(
        film_width, film_height, pos, dst, up, fov, lens_radius, focal_distance);
}

AGZ_TRACER_END
