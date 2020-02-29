#include <QSplitter>

#include <agz/editor/ui/left_panel.h>

AGZ_EDITOR_BEGIN

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent)
{
    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);

    up_panel        = new QWidget(this);
    up_panel_layout = new QVBoxLayout(up_panel);

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

    splitter->addWidget(up_panel);
    splitter->addWidget(down_tab);

    down_tab->addTab(material_tab,  "Material");
    down_tab->addTab(medium_tab,    "Medium");
    down_tab->addTab(geometry_tab,  "Geometry");
    down_tab->addTab(texture2d_tab, "Texture2D");
    down_tab->addTab(texture3d_tab, "Texture3D");

    up_panel->setContentsMargins(0, 0, 0, 0);
    up_panel_layout->setContentsMargins(0, 0, 0, 0);

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
    layout->addWidget(splitter);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

AGZ_EDITOR_END
