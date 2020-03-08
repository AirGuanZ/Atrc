#include <im3d_math.h>

#include <agz/editor/utility/direct_transform3d.h>

AGZ_EDITOR_BEGIN

// see https://www.gregslabaugh.net/publications/euler.pdf
Vec3 DirectTransform::to_euler_zyx(const Mat3 &m) noexcept
{
    if(std::abs(std::abs(m[0][2]) - 1) > tracer::EPS)
    {
        const real theta_1 = -std::asin(m[0][2]);
        const real cos_theta_1 = std::cos(theta_1);
        const real psi_1 = std::atan2(m[1][2] / cos_theta_1, m[2][2] / cos_theta_1);
        const real phi_1 = std::atan2(m[0][1] / cos_theta_1, m[0][0] / cos_theta_1);
        return { psi_1, theta_1, phi_1 };
    }

    const real phi = 0;
    if(std::abs(m[0][2] + 1) < tracer::EPS)
    {
        const real theta = PI_r / 2;
        const real psi = phi + std::atan2(m[1][0], m[2][0]);
        return { psi, theta, phi };
    }

    const real theta = -PI_r / 2;
    const real psi = -phi + std::atan2(-m[1][0], -m[2][0]);
    return { psi, theta, phi };
}

Mat3 DirectTransform::from_euler_zyx(const Vec3 &rad) noexcept
{
    return Mat3::rotate_z(rad.z) * Mat3::rotate_y(rad.y) * Mat3::rotate_x(rad.x);
}

Mat4 DirectTransform::compose() const noexcept
{
    Im3d::Mat3 rot_m;
    std::memcpy(&rot_m, &rotate, sizeof(Mat3));

    const Im3d::Mat4 m(
        { translate.x, translate.y, translate.z },
        rot_m,
        { scale, scale, scale });

    Mat4 ret;
    std::memcpy(&ret, &m, sizeof(Mat4));
    return ret;
}

std::shared_ptr<tracer::ConfigArray> DirectTransform::to_config() const
{
    auto trans_grp = std::make_shared<tracer::ConfigGroup>();
    trans_grp->insert_str("type", "translate");
    trans_grp->insert_child("offset", tracer::ConfigArray::from_vec3(translate));

    auto rot_grp = std::make_shared<tracer::ConfigGroup>();
    auto rot_m = std::make_shared<tracer::ConfigArray>();
    rot_grp->insert_str("type", "mat3");
    rot_m->push_back_real(rotate[0][0]);
    rot_m->push_back_real(rotate[0][1]);
    rot_m->push_back_real(rotate[0][2]);
    rot_m->push_back_real(rotate[1][0]);
    rot_m->push_back_real(rotate[1][1]);
    rot_m->push_back_real(rotate[1][2]);
    rot_m->push_back_real(rotate[2][0]);
    rot_m->push_back_real(rotate[2][1]);
    rot_m->push_back_real(rotate[2][2]);
    rot_grp->insert_child("col_major", rot_m);

    auto scale_grp = std::make_shared<tracer::ConfigGroup>();
    scale_grp->insert_str("type", "scale");
    scale_grp->insert_real("ratio", scale);

    auto arr = std::make_shared<tracer::ConfigArray>();
    arr->push_back(trans_grp);
    arr->push_back(rot_grp);
    arr->push_back(scale_grp);

    return arr;
}

AGZ_EDITOR_END
