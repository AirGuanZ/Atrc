#include <agz/tracer/core/light.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class NativeSky : public EnvirLight
{
    Spectrum top_;
    Spectrum bottom_;

    real user_specified_power_;

    Spectrum radiance_impl(const Vec3 &ref_to_light) const noexcept
    {
        const real cos_theta = math::clamp<real>(
            ref_to_light.normalize().z, -1, 1);
        
        const real s = (cos_theta + 1) / 2;
        return s * top_ + (1 - s) * bottom_;
    }

public:

    NativeSky(
        const Spectrum &top, const Spectrum &bottom,
        real user_specified_power)
    {
        top_    = top;
        bottom_ = bottom;
        user_specified_power_ = user_specified_power;
    }

    LightSampleResult sample(
        const Vec3 &ref, const Sample5 &sam) const noexcept override
    {
        const auto [dir, pdf] = math::distribution::uniform_on_sphere(sam.u, sam.v);

        return LightSampleResult(
            ref, emit_pos(ref, dir).pos, -dir, radiance_impl(dir), pdf);
    }

    real pdf(const Vec3 &ref, const Vec3 &) const noexcept override
    {
        return math::distribution::uniform_on_sphere_pdf<real>;
    }

    LightEmitResult sample_emit(const Sample5 &sam) const noexcept override
    {
        const auto [dir, pdf_dir] = math::distribution
                                        ::uniform_on_sphere(sam.u, sam.v);

        const Vec2 disk_sam   = math::distribution
                                    ::uniform_on_unit_disk(sam.w, sam.r);
        const Coord dir_coord = Coord::from_z(dir);
        const Vec3 pos        = world_radius_ *
            (disk_sam.x * dir_coord.x + disk_sam.y * dir_coord.y - dir) + world_centre_;

        return LightEmitResult(
            pos, dir, dir.normalize(), {}, radiance_impl(-dir),
            1 / (PI_r * world_radius_ * world_radius_), pdf_dir);
    }

    LightEmitPDFResult emit_pdf(
        const Vec3 &pos, const Vec3 &dir, const Vec3 &nor) const noexcept override
    {
        const real pdf_dir = math::distribution::uniform_on_sphere_pdf<real>;
        const real pdf_pos = 1 / (PI_r * world_radius_ * world_radius_);
        return { pdf_pos, pdf_dir };
    }

    LightEmitPosResult emit_pos(
        const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        // o: world_center
        // r: world_radius
        // x: ref
        // d: ref_to_light.normalize()
        // o + r * (u * ex + v * ey + d) = x + d * t
        // solve [u, v, t] and ans = ref + ref_to_light * t

        // => [a b c][u v t]^T = m
        // where a = r * ex
        //       b = r * ey
        //       c = -d
        //       m = x - o + r * d

        const auto [ex, ey, d] = Coord::from_z(ref_to_light);

        const Vec3 a = world_radius_ * ex;
        const Vec3 b = world_radius_ * ey;
        const Vec3 c = -d;
        const Vec3 m = ref - world_centre_ - world_radius_ * d;

        const real det  = Mat3::from_cols(a, b, c).det();
        const real tdet = Mat3::from_cols(a, b, m).det();

        if(std::abs(det) < EPS())
            return { world_centre_, -ref_to_light };

        const real t = tdet / det;
        const Vec3 pos = ref + t * d;

        return { pos, c };
    }

    Spectrum power() const noexcept override
    {
        if(user_specified_power_ > 0)
            return Spectrum(user_specified_power_);
        const real radius = world_radius_;
        const Spectrum mean_radiance = (top_ + bottom_) * real(0.5);
        return 4 * PI_r * PI_r * radius * radius * mean_radiance;
    }

    Spectrum radiance(
        const Vec3 &ref, const Vec3 &ref_to_light) const noexcept override
    {
        return radiance_impl(ref_to_light);
    }
};

RC<EnvirLight> create_native_sky(
    const Spectrum &top,
    const Spectrum &bottom,
    real user_specified_power)
{
    return newRC<NativeSky>(top, bottom, user_specified_power);
}

AGZ_TRACER_END
