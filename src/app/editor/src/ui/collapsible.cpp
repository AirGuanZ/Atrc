#include <QPropertyAnimation>

#include <agz/editor/ui/collapsible.h>

AGZ_EDITOR_BEGIN

Collapsible::Collapsible(QWidget *parent, const QString &title, int ani_duration_ms)
    : QWidget(parent), ani_duration_ms_(ani_duration_ms)
{
    toggle_button_ = new QToolButton(this);
    animation_     = new QParallelAnimationGroup(this);
    content_       = new QScrollArea(this);
    layout_        = new QGridLayout(this);

    toggle_button_->setStyleSheet("QToolButton { border: none; }");
    toggle_button_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggle_button_->setArrowType(Qt::ArrowType::RightArrow);
    toggle_button_->setText(title);
    toggle_button_->setCheckable(true);
    toggle_button_->setChecked(false);

    content_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    content_->setFrameShape(QFrame::NoFrame);

    content_->setMaximumHeight(0);
    content_->setMinimumHeight(0);

    animation_->addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    animation_->addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    animation_->addAnimation(new QPropertyAnimation(content_, "maximumHeight"));

    layout_->setVerticalSpacing(0);
    layout_->setContentsMargins(0, 0, 0, 0);

    layout_->addWidget(toggle_button_, 0, 0, 1, 1, Qt::AlignLeft);
    layout_->addWidget(content_, 1, 0, 1, 1);

    connect(toggle_button_, &QToolButton::toggled, this, &Collapsible::toggle);
}

void Collapsible::set_content_layout(QLayout *content_layout)
{
    delete content_->layout();
    content_->setLayout(content_layout);
}

void Collapsible::toggle(bool collapsed)
{
    const int collapsed_height = sizeHint().height() - content_->maximumHeight();
    const int content_height = content_->layout()->sizeHint().height();

    for(int i = 0; i < animation_->animationCount() - 1; ++i)
    {
        QPropertyAnimation *sec_ani = static_cast<QPropertyAnimation *>(animation_->animationAt(i));
        sec_ani->setDuration(ani_duration_ms_);
        sec_ani->setStartValue(collapsed_height);
        sec_ani->setEndValue(collapsed_height + content_height);
    }

    QPropertyAnimation *content_ani = static_cast<QPropertyAnimation *>(
        animation_->animationAt(animation_->animationCount() - 1));
    content_ani->setDuration(ani_duration_ms_);
    content_ani->setStartValue(0);
    content_ani->setEndValue(content_height);

    toggle_button_->setArrowType(collapsed ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
    animation_->setDirection(collapsed ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    animation_->start();
}

AGZ_EDITOR_END
