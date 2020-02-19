#pragma once

#include <QLayout>
#include <QStyle>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

// Similiar to https://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html
class FlowLayout : public QLayout
{
public:

    explicit FlowLayout(QWidget *parent = nullptr, int margin = -1, int hori_spacing = -1, int vert_spacing = -1);

    ~FlowLayout();

    void addItem(QLayoutItem *item) override;

    Qt::Orientations expandingDirections() const override;

    bool hasHeightForWidth() const override;

    int heightForWidth(int width) const override;

    int count() const override;

    QLayoutItem *itemAt(int index) const override;

    QSize minimumSize() const override;

    void setGeometry(const QRect &rect) override;

    QSize sizeHint() const override;

    QLayoutItem *takeAt(int index) override;

private:

    int do_layout(const QRect &rect, bool test_only) const;

    int smart_spacing(QStyle::PixelMetric pixel_metric) const;

    int horizontal_spacing() const;

    int vertical_spacing() const;

    QList<QLayoutItem *> items_;

    int hori_space_;
    int vert_space_;
};

AGZ_EDITOR_END
