#include <agz/editor/ui/right_panel.h>

AGZ_EDITOR_BEGIN

RightPanel::RightPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(right_tab);

    resource_tab_layout = new QVBoxLayout(resource_tab);
    renderer_tab_layout = new QVBoxLayout(renderer_tab);
    camera_tab_layout   = new QVBoxLayout(camera_tab);

    resource_tab_layout->setAlignment(Qt::AlignTop);
    renderer_tab_layout->setAlignment(Qt::AlignTop);
    camera_tab_layout  ->setAlignment(Qt::AlignTop);
}

AGZ_EDITOR_END
