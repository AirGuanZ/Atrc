#include <QWidget>

#include <agz/editor/ui/utility/flow_layout.h>

AGZ_EDITOR_BEGIN

FlowLayout::FlowLayout(
    QWidget *parent, int margin, int hori_spacing, int vert_spacing)
    : QLayout(parent), hori_space_(hori_spacing), vert_space_(vert_spacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    while(!items_.empty())
        delete items_.takeAt(0);
}

void FlowLayout::addItem(QLayoutItem *item)
{
    items_.append(item);
}

int FlowLayout::count() const
{
    return items_.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return items_.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if(0 <= index && index < items_.size())
        return items_.takeAt(index);
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return 0;
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return do_layout(QRect(0, 0, width, 0), true);
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    (void)do_layout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for(auto item : items_)
        size = size.expandedTo(item->minimumSize());

    size += QSize(2 * margin(), 2 * margin());
    return size;
}

int FlowLayout::do_layout(const QRect &rect, bool test_only) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    const QRect effective_rect = rect.adjusted(left, top, -right, -bottom);
    int current_x = effective_rect.x();
    int current_y = effective_rect.y();

    int line_height = 0;
    for(auto item : items_)
    {
        QWidget *widget = item->widget();

        int space_x = horizontal_spacing();
        if(space_x == -1)
        {
            space_x = widget->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        }

        int space_y = vertical_spacing();
        if(space_y == -1)
        {
            space_y = widget->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }

        int next_x = current_x + item->sizeHint().width() + space_x;
        if(next_x - space_x > effective_rect.right() && line_height > 0)
        {
            current_x = effective_rect.x();
            current_y += line_height + space_y;
            next_x = current_x + item->sizeHint().width() + space_x;
            line_height = 0;
        }

        if(!test_only)
            item->setGeometry(QRect(QPoint(current_x, current_y), item->sizeHint()));

        current_x = next_x;
        line_height = (std::max)(line_height, item->sizeHint().height());
    }

    return current_y + line_height - rect.y() + bottom;
}

int FlowLayout::smart_spacing(QStyle::PixelMetric pixel_metric) const
{
    QObject *parent = this->parent();
    if(!parent)
        return -1;

    if(parent->isWidgetType())
    {
        QWidget *widget = static_cast<QWidget *>(parent);
        return widget->style()->pixelMetric(pixel_metric, nullptr, widget);
    }

    return static_cast<QLayout *>(parent)->spacing();
}

int FlowLayout::horizontal_spacing() const
{
    if(hori_space_ >= 0)
        return hori_space_;
    return smart_spacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::vertical_spacing() const
{
    if(vert_space_)
        return vert_space_;
    return smart_spacing(QStyle::PM_LayoutVerticalSpacing);
}

AGZ_EDITOR_END
