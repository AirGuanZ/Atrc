#include <agz/editor/ui/down_panel.h>

AGZ_EDITOR_BEGIN

DownPanel::DownPanel(QWidget *parent)
    : QWidget(parent)
{
    down_tab      = new QTabWidget(this);
    material_tab  = new QWidget();
    medium_tab    = new QWidget();
    geometry_tab  = new QWidget();
    texture2d_tab = new QWidget();
    texture3d_tab = new QWidget();

    material_tab_layout  = new QVBoxLayout(material_tab);
    medium_tab_layout    = new QVBoxLayout(medium_tab);
    geometry_tab_layout  = new QVBoxLayout(geometry_tab);
    texture2d_tab_layout = new QVBoxLayout(texture2d_tab);
    texture3d_tab_layout = new QVBoxLayout(texture3d_tab);

    down_tab->addTab(material_tab,  "Material");
    down_tab->addTab(medium_tab,    "Medium");
    down_tab->addTab(geometry_tab,  "Geometry");
    down_tab->addTab(texture2d_tab, "Texture2D");
    down_tab->addTab(texture3d_tab, "Texture3D");

    material_tab->setContentsMargins(0, 0, 0, 0);
    material_tab_layout->setContentsMargins(0, 0, 0, 0);

    medium_tab->setContentsMargins(0, 0, 0, 0);
    medium_tab_layout->setContentsMargins(0, 0, 0, 0);

    geometry_tab->setContentsMargins(0, 0, 0, 0);
    geometry_tab_layout->setContentsMargins(0, 0, 0, 0);

    texture2d_tab->setContentsMargins(0, 0, 0, 0);
    texture2d_tab_layout->setContentsMargins(0, 0, 0, 0);

    texture3d_tab->setContentsMargins(0, 0, 0, 0);
    texture3d_tab_layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(down_tab);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

AGZ_EDITOR_END
