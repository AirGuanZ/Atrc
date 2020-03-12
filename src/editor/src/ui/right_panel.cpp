#include <agz/editor/ui/right_panel.h>

AGZ_EDITOR_BEGIN

RightPanel::RightPanel(QWidget *parent)
    : QWidget(parent)
{
    right_tab_widget = new QTabWidget(this);
    right_tab_widget->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Expanding);

    entity_area      = new QScrollArea;
    resource_area    = new QScrollArea;
    renderer_area    = new QScrollArea;
    envir_light_area = new QScrollArea;
    camera_area      = new QScrollArea;

    QWidget *global_tab = new QWidget;

    entity_tab      = new QWidget;
    resource_tab    = new QWidget;
    renderer_tab    = new QWidget;
    envir_light_tab = new QWidget;
    camera_tab      = new QWidget;

    entity_tab     ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resource_tab   ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    renderer_tab   ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    envir_light_tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    camera_tab     ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    entity_area     ->setWidget(entity_tab);
    resource_area   ->setWidget(resource_tab);
    renderer_area   ->setWidget(renderer_tab);
    envir_light_area->setWidget(envir_light_tab);
    camera_area     ->setWidget(camera_tab);

    entity_area     ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    resource_area   ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    renderer_area   ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    envir_light_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    camera_area     ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    entity_area     ->setWidgetResizable(true);
    resource_area   ->setWidgetResizable(true);
    renderer_area   ->setWidgetResizable(true);
    envir_light_area->setWidgetResizable(true);
    camera_area     ->setWidgetResizable(true);

    QVBoxLayout *global_tab_layout = new QVBoxLayout(global_tab);
    QTabWidget  *global_tab_widget = new QTabWidget(global_tab);

    global_tab_layout->addWidget(global_tab_widget);
    global_tab_widget->addTab(renderer_area,    "Renderer");
    global_tab_widget->addTab(envir_light_area, "Environment");
    global_tab_widget->addTab(camera_area,      "Camera");

    global_tab->setContentsMargins(0, 0, 0, 0);
    global_tab_layout->setContentsMargins(0, 0, 0, 0);

    right_tab_widget->addTab(entity_area,   "Entity");
    right_tab_widget->addTab(resource_area, "Resource");
    right_tab_widget->addTab(global_tab,    "Global");

    entity_tab_layout      = new QVBoxLayout(entity_tab);
    resource_tab_layout    = new QVBoxLayout(resource_tab);
    renderer_tab_layout    = new QVBoxLayout(renderer_tab);
    envir_light_tab_layout = new QVBoxLayout(envir_light_tab);
    camera_tab_layout      = new QVBoxLayout(camera_tab);
    
    entity_tab_layout     ->setAlignment(Qt::AlignTop);
    resource_tab_layout   ->setAlignment(Qt::AlignTop);
    renderer_tab_layout   ->setAlignment(Qt::AlignTop);
    envir_light_tab_layout->setAlignment(Qt::AlignTop);
    camera_tab_layout     ->setAlignment(Qt::AlignTop);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(right_tab_widget);
}

AGZ_EDITOR_END
