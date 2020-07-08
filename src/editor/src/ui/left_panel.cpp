#include <QSplitter>

#include <agz/editor/ui/left_panel.h>

AGZ_EDITOR_BEGIN

LeftPanel::LeftPanel(QWidget *parent)
    : QWidget(parent)
{
    up_panel        = new QWidget(this);
    up_panel_layout = new QVBoxLayout(up_panel);

    up_panel->setContentsMargins(0, 0, 0, 0);
    up_panel_layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(up_panel);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
}

AGZ_EDITOR_END
