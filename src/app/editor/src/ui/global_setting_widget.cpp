#include <QVBoxLayout>

#include <agz/editor/ui/global_setting_widget.h>

AGZ_EDITOR_BEGIN

GlobalSettingWidget::GlobalSettingWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(gridLayoutWidget);
}

AGZ_EDITOR_END
