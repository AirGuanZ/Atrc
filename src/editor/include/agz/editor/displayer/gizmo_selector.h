#pragma once

#include <QPushButton>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class GizmoSelector : public QWidget
{
    Q_OBJECT

public:

    enum class GizmoType
    {
        None,
        Translate,
        Rotate,
        Scale
    };

    GizmoSelector();

    GizmoType get_gizmo_type() const noexcept;

    bool is_local() const noexcept;

signals:

    void change_gizmo();

private:

    QPushButton *translate_ = nullptr;
    QPushButton *rotate_    = nullptr;
    QPushButton *scale_     = nullptr;

    QPushButton *local_ = nullptr;
};

AGZ_EDITOR_END
