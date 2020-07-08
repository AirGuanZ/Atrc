#include <agz/tracer/core/camera.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class ThinLensCamera : public Camera
{
    FVec3 pos_;
    FVec3 dir_;

    real focal_film_width_  = 1;
    real focal_film_height_ = 1;

    real area_focal_film_ = 1;
    real area_lens_       = 1;

    real lens_radius_ = 0;
    real focal_distance_ = 1;

    FTransform3 camera_to_world_;

    struct Params
    {
        real film_aspect    = 1;
        FVec3 pos;
        FVec3 dst;
        FVec3 up;
        real fov            = 0;
        real lens_radius    = 0;
        real focal_distance = 0;

    } params_;

    void init_from_params(const Params &params)
    {
        const real aspect = params.film_aspect;

        focal_film_height_ = 2 * params.focal_distance
                               * std::tan(params.fov / 2);
        focal_film_width_ = aspect * focal_film_height_;

        area_focal_film_ = focal_film_width_ * focal_film_height_;
        area_lens_ = params.lens_radius > 0 ?
                     PI_r * params.lens_radius * params.lens_radius : 1;

        camera_to_world_ = FTransform3(
            FTrans4::look_at(
                params.pos, params.dst, params.up)).inv();

        pos_ = params.pos;
        dir_ = (params.dst - params.pos).normalize();

        lens_radius_ = params.lens_radius;
        focal_distance_ = params.focal_distance;
    }

public:

    ThinLensCamera(
        real film_aspect,
        const FVec3 &pos, const FVec3 &dst, const FVec3 &up,
        real fov, real lens_radius, real focal_distance)
    {
        params_.film_aspect    = film_aspect;
        params_.pos            = pos;
        params_.dst            = dst;
        params_.up             = up;
        params_.fov            = fov;
        params_.lens_radius    = lens_radius;
        params_.focal_distance = focal_distance;
        init_from_params(params_);
    }

    CameraSampleWeResult sample_we(
        const Vec2 &film_coord,
        const Sample2 &aperture_sample) const noexcept override
    {
        const FVec3 focal_film_pos = {
            (real(0.5) - film_coord.x) * focal_film_width_,
            (film_coord.y - real(0.5)) * focal_film_height_,
            focal_distance_
        };
        const Vec2 disk_sam = math::distribution::uniform_on_unit_disk(
            aperture_sample.u, aperture_sample.v);
        const FVec3 lens_pos = FVec3(
            lens_radius_ * disk_sam.x, lens_radius_ * disk_sam.y, 0);

        const FVec3 pos_on_cam = camera_to_world_.apply_to_point(lens_pos);;
        const FVec3 pos_to_out = camera_to_world_.apply_to_vector(
            focal_film_pos - lens_pos).normalize();
        
        return CameraSampleWeResult(
            pos_on_cam, pos_to_out, dir_, Spectrum(1));
    }

    CameraEvalWeResult eval_we(
        const FVec3 &pos_on_cam, const FVec3 &pos_to_out) const noexcept override
    {
        const FVec3 lens_pos  = camera_to_world_.apply_inverse_to_point(pos_on_cam);
        const FVec3 local_dir = camera_to_world_
            .apply_inverse_to_vector(pos_to_out)
            .normalize();

        if(local_dir.z <= 0)
            return CAMERA_EVAL_WE_RESULT_ZERO;

        const FVec3 focal_film_pos =
            lens_pos + (focal_distance_ / local_dir.z) * local_dir;

        const Vec2 film_coord = {
            real(0.5) - focal_film_pos.x / focal_film_width_,
            real(0.5) + focal_film_pos.y / focal_film_height_
        };

        const real cos_theta  = local_dir.z;
        const real cos2_theta = cos_theta * cos_theta;
        const real we         = focal_distance_ * focal_distance_
                              / (area_focal_film_ * area_lens_
                               * cos2_theta * cos2_theta);

        return { Spectrum(we), film_coord, dir_ };
    }

    CameraWePDFResult pdf_we(
        const FVec3 &pos_on_cam, const FVec3 &pos_to_out) const noexcept override
    {
        const FVec3 local_dir = camera_to_world_
            .apply_inverse_to_vector(pos_to_out)
            .normalize();

        if(local_dir.z <= 0)
            return { 1 / area_lens_, 0 };

        const real cos_theta = local_dir.z;
        const real pdf_dir   = focal_distance_ * focal_distance_
                             / (area_focal_film_ * cos_theta
                              * cos_theta * cos_theta);

        return { 1 / area_lens_, pdf_dir };
    }

    CameraSampleWiResult sample_wi(
        const FVec3 &ref, const Sample2 &sam) const noexcept override
    {
        const FVec3 local_ref = camera_to_world_.apply_inverse_to_point(ref);

        // sample lens

        const Vec2 disk_sam = math::distribution::uniform_on_unit_disk(
            sam.u, sam.v);

        const Vec2 lens_sam = lens_radius_ * disk_sam;
        const FVec3 lens_pos = { lens_sam.x, lens_sam.y, 0 };

        // compute local dir

        const FVec3 local_dir = (local_ref - lens_pos).normalize();
        if(local_dir.z <= 0)
            return CAMERA_SAMPLE_WI_RESULT_INVALID;

        // compute film coord

        const FVec3 focal_film_pos = lens_pos
                                  + (focal_distance_ / local_dir.z) * local_dir;
        const Vec2 film_coord = {
            real(0.5) - focal_film_pos.x / focal_film_width_,
            real(0.5) + focal_film_pos.y / focal_film_height_
        };

        const FVec3 pos_on_cam = camera_to_world_.apply_to_point(lens_pos);
        const FVec3 nor_at_pos = dir_;
        const FVec3 ref_to_pos = pos_on_cam - ref;
        const Spectrum we = eval_we(pos_on_cam, -ref_to_pos).we;
        const real pdf = (pos_on_cam - ref).length_square()
                       / (local_dir.z * area_lens_);

        return CameraSampleWiResult(
            pos_on_cam, nor_at_pos, ref_to_pos, we, pdf, film_coord);
    }
};

RC<Camera> create_thin_lens_camera(
    real film_aspect,
    const FVec3 &pos,
    const FVec3 &dst,
    const FVec3 &up,
    real fov,
    real lens_radius,
    real focal_distance)
{
    return newRC<ThinLensCamera>(
        film_aspect, pos, dst, up, fov, lens_radius, focal_distance);
}

AGZ_TRACER_END
