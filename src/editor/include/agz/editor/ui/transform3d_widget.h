#pragma once

#include <agz/editor/ui/utility/vec_input.h>
#include <agz/editor/utility/direct_transform3d.h>

AGZ_EDITOR_BEGIN

class Transform3DWidget : public QWidget
{
    Q_OBJECT

public:

    explicit Transform3DWidget(const DirectTransform &init_data = {});

    DirectTransform get_transform() const;

    void set_transform(const DirectTransform &transform);

signals:

    void edit_transform();

private:

    Mat3 rotate_ = Mat3::identity();

    Vec3Input *translate_ = nullptr;
    Vec3Input *euler_zyx_ = nullptr;
    RealInput *scale_     = nullptr;
};

AGZ_EDITOR_END
