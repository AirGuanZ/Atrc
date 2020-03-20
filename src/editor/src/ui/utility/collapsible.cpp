#include <QApplication>

#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

Collapsible::Collapsible(QWidget *parent, const QString &title)
    : QWidget(parent)
{
    toggle_button_ = new QToolButton(this);
    layout_        = new QVBoxLayout(this);
    
    toggle_button_->setStyleSheet("QToolButton { border: none; }");
    toggle_button_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggle_button_->setArrowType(Qt::ArrowType::RightArrow);
    toggle_button_->setText(title);
    toggle_button_->setCheckable(true);
    toggle_button_->setChecked(false);

    layout_->addWidget(toggle_button_, 0, Qt::AlignLeft);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);

    connect(toggle_button_, &QToolButton::toggled, this, &Collapsible::toggle);
}

void Collapsible::set_content_widget(QWidget *widget)
{
    if(content_)
        delete content_;

    content_ = widget;
    layout_->addWidget(content_);

    const auto cm = content_->contentsMargins();
    content_->setContentsMargins(
        (std::max)(cm.left(), toggle_button_->iconSize().width() + 5),
        cm.top(), cm.right(), cm.bottom());

    content_->hide();
}

void Collapsible::open()
{
    toggle_button_->setChecked(true);
}

void Collapsible::set_toggle_button_disabled(bool disabled)
{
    toggle_button_->setDisabled(disabled);
}

void Collapsible::toggle(bool checked)
{
    if(!content_)
        return;

    if(checked)
    {
        toggle_button_->setArrowType(Qt::ArrowType::DownArrow);
        content_->show();
    }
    else
    {
        toggle_button_->setArrowType(Qt::ArrowType::RightArrow);
        content_->hide();
    }
}

AGZ_EDITOR_END
