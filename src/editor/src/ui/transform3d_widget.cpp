#include <QGridLayout>
#include <QLabel>
#include <QInputDialog>
#include <QPushButton>

#include <im3d_math.h>

#include <agz/editor/ui/transform3d_widget.h>

AGZ_EDITOR_BEGIN

Transform3DWidget::Transform3DWidget(const DirectTransform &init_data)
{
    translate_ = new Vec3Input(this);
    euler_zyx_ = new Vec3Input(this);
    scale_     = new RealInput(this);

    rotate_ = init_data.rotate;

    translate_->set_value(init_data.translate);
    euler_zyx_->set_value(DirectTransform::to_euler_zyx(rotate_).map(math::rad2deg<real>));
    scale_    ->set_value(init_data.scale);

    QPushButton *clear_button = new QPushButton("Clear Transform", this);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(new QLabel("Translate", this), 0, 0);
    layout->addWidget(translate_, 0, 1);
    layout->addWidget(new QLabel("Rotate",    this), 1, 0);
    layout->addWidget(euler_zyx_, 1, 1);
    layout->addWidget(new QLabel("Scale",     this), 2, 0);
    layout->addWidget(scale_,     2, 1);
    layout->addWidget(clear_button, 3, 0, 1, 2);

    connect(translate_, &Vec3Input::edit_value, [=](const Vec3 &)
    {
        emit edit_transform();
    });

    connect(euler_zyx_, &Vec3Input::edit_value, [=](const Vec3 &new_zyx)
    {
        rotate_ = DirectTransform::from_euler_zyx(new_zyx.map(math::deg2rad<real>));
        emit edit_transform();
    });

    connect(scale_, &RealInput::edit_value, [=](real)
    {
        emit edit_transform();
    });

    connect(clear_button, &QPushButton::clicked, [=]
    {
        rotate_ = Mat3::identity();

        translate_->set_value({ 0, 0, 0 });
        euler_zyx_->set_value({ 0, 0, 0 });
        scale_    ->set_value(1);

        emit edit_transform();
    });
}

DirectTransform Transform3DWidget::get_transform() const
{
    DirectTransform ret;
    ret.rotate = rotate_;
    ret.translate = translate_->get_value();
    ret.scale     = scale_    ->get_value();
    return ret;
}

void Transform3DWidget::set_transform(const DirectTransform &transform)
{
    rotate_ = transform.rotate;
    translate_->set_value(transform.translate);
    euler_zyx_->set_value(DirectTransform::to_euler_zyx(rotate_).map(math::rad2deg<real>));
    scale_    ->set_value(transform.scale);
}

AGZ_EDITOR_END
