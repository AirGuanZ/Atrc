#include <QHBoxLayout>

#include <agz/editor/displayer/gizmo_selector.h>

AGZ_EDITOR_BEGIN

GizmoSelector::GizmoSelector()
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    translate_ = new QPushButton("T", this);
    rotate_    = new QPushButton("R", this);
    scale_     = new QPushButton("S", this);
    local_     = new QPushButton("Local", this);

    translate_->setCheckable(true);
    rotate_   ->setCheckable(true);
    scale_    ->setCheckable(true);
    local_    ->setCheckable(true);

    translate_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    rotate_   ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    scale_    ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    local_    ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    layout->addWidget(translate_);
    layout->addWidget(rotate_);
    layout->addWidget(scale_);
    layout->addWidget(local_);

    connect(translate_, &QPushButton::clicked, [=]
    {
        if(translate_->isChecked())
        {
            rotate_->setChecked(false);
            scale_ ->setChecked(false);
        }
        emit change_gizmo();
    });

    connect(rotate_, &QPushButton::clicked, [=]
    {
        if(rotate_->isChecked())
        {
            translate_->setChecked(false);
            scale_    ->setChecked(false);
        }
        emit change_gizmo();
    });

    connect(scale_, &QPushButton::clicked, [=]
    {
        if(scale_->isChecked())
        {
            translate_->setChecked(false);
            rotate_   ->setChecked(false);
        }
        emit change_gizmo();
    });

    connect(local_, &QPushButton::clicked, [=]
    {
        emit change_gizmo();
    });
}

GizmoSelector::GizmoType GizmoSelector::get_gizmo_type() const noexcept
{
    if(translate_->isChecked())
        return GizmoType::Translate;
    if(rotate_->isChecked())
        return GizmoType::Rotate;
    if(scale_->isChecked())
        return GizmoType::Scale;
    return GizmoType::None;
}

bool GizmoSelector::is_local() const noexcept
{
    return local_->isChecked();
}

AGZ_EDITOR_END
