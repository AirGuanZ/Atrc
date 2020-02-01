#include <agz/editor/displayer/camera_panel.h>

AGZ_EDITOR_BEGIN

CameraPanel::CameraPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(gridLayoutWidget);

    constexpr double L = std::numeric_limits<double>::lowest();
    constexpr double H = std::numeric_limits<double>::max();

    position_x->setRange(L, H);
    position_y->setRange(L, H);
    position_z->setRange(L, H);

    destination_x->setRange(L, H);
    destination_y->setRange(L, H);
    destination_z->setRange(L, H);

    up_x->setRange(L, H);
    up_y->setRange(L, H);
    up_z->setRange(L, H);

    fov           ->setRange(0, H);
    lens_radius   ->setRange(0, H);
    focal_distance->setRange(0, H);
}

AGZ_EDITOR_END
