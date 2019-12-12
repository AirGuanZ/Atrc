#include <agz/tracer/core/camera.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class ThinLensCamera : public Camera
{
    Vec2 film_sample_low_;  // 以film coord为单位的采样范围
    Vec2 film_sample_high_;

    Vec3 pos_;
    Vec3 dir_;

    real focal_film_width_  = 1;
    real focal_film_height_ = 1;

    real area_focal_film_ = 1;
    real area_lens_       = 1;

    real lens_radius_ = 0;
    real focal_distance_ = 1;

    Transform3 camera_to_world_;

    bool is_film_coord_in_sample_bound(const Vec2 &film_coord) const noexcept
    {
        return film_sample_low_.x <= film_coord.x && film_coord.x <= film_sample_high_.x &&
               film_sample_low_.y <= film_coord.y && film_coord.y <= film_sample_high_.y;
    }

public:

    using Camera::Camera;

    void initialize(
        const Vec3 &pos, const Vec3 &dst, const Vec3 &up,
        real fov, real aspect,
        real lens_radius, real focal_distance)
    {
        AGZ_HIERARCHY_TRY

        film_sample_low_  = film_->sample_low();
        film_sample_high_ = film_->sample_high();

        film_sample_low_.x  /= film_->resolution().x;
        film_sample_high_.x /= film_->resolution().x;
        
        film_sample_low_.y  /= film_->resolution().y;
        film_sample_high_.y /= film_->resolution().y;

        focal_film_height_ = 2 * focal_distance * std::tan(fov / 2);
        focal_film_width_  = aspect * focal_film_height_;

        area_focal_film_ = focal_film_width_ * focal_film_height_ * (film_sample_high_ - film_sample_low_).product();
        area_lens_       = lens_radius > 0 ? PI_r * lens_radius * lens_radius : 1;

        camera_to_world_ = Transform3(Trans4::look_at(pos, dst, up)).inv();

        pos_  = pos;
        dir_  = (dst - pos).normalize();
        
        lens_radius_    = lens_radius;
        focal_distance_ = focal_distance;

        AGZ_HIERARCHY_WRAP("in initializing pinhole camera")
    }

    CameraSampleWeResult sample_we(const Sample2 &film_coord, const Sample2 &aperture_sample) const noexcept override
    {
        Vec3 focal_film_pos = {
            (real(0.5) - film_coord.u) * focal_film_width_,
            (film_coord.v - real(0.5)) * focal_film_height_,
            focal_distance_
        };
        Vec2 disk_sam = math::distribution::uniform_on_unit_disk(aperture_sample.u, aperture_sample.v);
        Vec3 lens_pos = Vec3(lens_radius_ * disk_sam.x, lens_radius_ * disk_sam.y, 0);

        CameraSampleWeResult ret;
        ret.pos_on_cam = camera_to_world_.apply_to_point(lens_pos);
        ret.pos_to_out = camera_to_world_.apply_to_vector(focal_film_pos - lens_pos).normalize();
        ret.throughput = Spectrum(1);

        return ret;
    }

    CameraEvalWeResult eval_we(const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept override
    {
        // 若方向和摄像机朝向差距超过180度，丢弃

        Vec3 lens_pos  = camera_to_world_.apply_inverse_to_point(pos_on_cam);
        Vec3 local_dir = camera_to_world_.apply_inverse_to_vector(pos_to_out).normalize();
        if(local_dir.z <= 0)
            return CAMERA_EVAL_WE_RESULT_ZERO;

        // 计算film coord

        Vec3 focal_film_pos = lens_pos + (focal_distance_ / local_dir.z) * local_dir;

        // 丢弃film coord超出采样范围的结果

        Vec2 film_coord = {
            real(0.5) - focal_film_pos.x / focal_film_width_,
            real(0.5) + focal_film_pos.y / focal_film_height_
        };
        if(!is_film_coord_in_sample_bound(film_coord))
            return CAMERA_EVAL_WE_RESULT_ZERO;

        real cos_theta  = local_dir.z;
        real cos2_theta = cos_theta * cos_theta;
        real we         = focal_distance_ * focal_distance_ / (area_focal_film_ * area_lens_ * cos2_theta * cos2_theta);

        return { Spectrum(we), film_coord };
    }

    CameraWePDFResult pdf_we(const Vec3 &pos_on_cam, const Vec3 &pos_to_out) const noexcept override
    {
        // 若方向和摄像机朝向差距超过180度，则方向pdf为0

        Vec3 lens_pos  = camera_to_world_.apply_inverse_to_point(pos_on_cam);
        Vec3 local_dir = camera_to_world_.apply_inverse_to_vector(pos_to_out).normalize();
        if(local_dir.z <= 0)
            return { 1 / area_lens_, 0 };

        // 若film coord超出采样范围，则方向pdf为0

        Vec3 focal_film_pos = lens_pos + (focal_distance_ / local_dir.z) * local_dir;
        Vec2 film_coord = {
            real(0.5) - focal_film_pos.x / focal_film_width_,
            real(0.5) + focal_film_pos.y / focal_film_height_
        };
        if(!is_film_coord_in_sample_bound(film_coord))
            return { 1 / area_lens_, 0 };

        real cos_theta = local_dir.z;
        real pdf_dir   = focal_distance_ * focal_distance_ / (area_focal_film_ * cos_theta * cos_theta * cos_theta);

        return { 1 / area_lens_, pdf_dir };
    }

    CameraSampleWiResult sample_wi(const Vec3 &ref, const Sample2 &sam) const noexcept override
    {
        Vec3 local_ref = camera_to_world_.apply_inverse_to_point(ref);

        // 采样镜头

        Vec2 disk_sam = math::distribution::uniform_on_unit_disk(sam.u, sam.v);
        Vec2 lens_sam = lens_radius_ * disk_sam;
        Vec3 lens_pos = { lens_sam.x, lens_sam.y, 0 };

        // 计算方向

        Vec3 local_dir = (local_ref - lens_pos).normalize();
        if(local_dir.z <= 0)
            return CAMERA_SAMPLE_WI_RESULT_INVALID;

        // 计算focal film上的位置

        Vec3 focal_film_pos = lens_pos + (focal_distance_ / local_dir.z) * local_dir;
        Vec2 film_coord = {
            real(0.5) - focal_film_pos.x / focal_film_width_,
            real(0.5) + focal_film_pos.y / focal_film_height_
        };
        if(!is_film_coord_in_sample_bound(film_coord))
            return CAMERA_SAMPLE_WI_RESULT_INVALID;

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
        AABB bound;
        bound |= pos_;

        bound |= camera_to_world_.apply_to_point({ -lens_radius_, -lens_radius_, 0 });
        bound |= camera_to_world_.apply_to_point({ -lens_radius_, +lens_radius_, 0 });
        bound |= camera_to_world_.apply_to_point({ +lens_radius_, -lens_radius_, 0 });
        bound |= camera_to_world_.apply_to_point({ +lens_radius_, +lens_radius_, 0 });

        real w = focal_film_width_ / 2, h = focal_film_height_ / 2;
        bound |= camera_to_world_.apply_to_point({ -w, -h, focal_distance_ });
        bound |= camera_to_world_.apply_to_point({ -w, +h, focal_distance_ });
        bound |= camera_to_world_.apply_to_point({ +w, -h, focal_distance_ });
        bound |= camera_to_world_.apply_to_point({ +w, +h, focal_distance_ });

        return bound;
    }
};

std::shared_ptr<Camera> create_thin_lens_camera(
    std::shared_ptr<const Film> film,
    const Vec3 &pos,
    const Vec3 &dst,
    const Vec3 &up,
    real fov,
    real aspect,
    real lens_radius,
    real focal_distance)
{
    auto ret = std::make_shared<ThinLensCamera>(std::move(film));
    ret->initialize(pos, dst, up, fov,aspect, lens_radius, focal_distance);
    return ret;
}

AGZ_TRACER_END
