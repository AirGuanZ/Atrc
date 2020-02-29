#include <QInputDialog>
#include <QLabel>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/ui/transform2d_widget.h>
#include <agz/editor/ui/utility/validator.h>

AGZ_EDITOR_BEGIN

namespace
{
    class Translate2DWidget : public Transform2DUnitWidget
    {
    public:

        explicit Translate2DWidget(QWidget *parent, const Vec2 &offset = {})
            : Transform2DUnitWidget(parent)
        {
            QHBoxLayout *layout    = new QHBoxLayout(this);
            QLabel      *text      = new QLabel("Translate Offset", this);
            QPushButton *rm_button = new QPushButton("-", this);

            text->setAlignment(Qt::AlignCenter);

            edit_validator_ = std::make_unique<Vec2Validator>();

            offset_edit_ = new QLineEdit(this);
            offset_edit_->setText(QString("%1 %2").arg(offset.x).arg(offset.y));
            offset_edit_->setAlignment(Qt::AlignCenter);
            offset_edit_->setValidator(edit_validator_.get());

            layout->addWidget(rm_button);
            layout->addWidget(text);
            layout->addWidget(offset_edit_);

            connect(rm_button, &QPushButton::clicked, [=]
            {
                emit remove_this();
            });

            connect(offset_edit_, &QLineEdit::returnPressed, [=]
            {
                emit change_transform();
            });

            setContentsMargins(0, 0, 0, 0);
            layout->setContentsMargins(0, 0, 0, 0);
        }

        UnitType get_type() const noexcept override
        {
            return UnitType::Translate;
        }

        tracer::Transform2 get_transform() const override
        {
            real x, y;
            QString line = offset_edit_->text();
            QTextStream(&line) >> x >> y;
            return tracer::Transform2::translate(x, y);
        }

        Transform2DUnitWidget *clone() const override
        {
            real x, y;
            QString line = offset_edit_->text();
            QTextStream(&line) >> x >> y;
            return new Translate2DWidget(nullptr, { x, y });
        }

        void save_asset(AssetSaver &saver) const override
        {
            saver.write_string(offset_edit_->text());
        }

        void load_asset(AssetLoader &loader) override
        {
            offset_edit_->setText(loader.read_string());
        }

    private:

        QLineEdit *offset_edit_;

        std::unique_ptr<Vec2Validator> edit_validator_;
    };

    class Rotate2DWidget : public Transform2DUnitWidget
    {
    public:

        explicit Rotate2DWidget(QWidget *parent, real deg = 0)
            : Transform2DUnitWidget(parent)
        {
            QHBoxLayout *layout    = new QHBoxLayout(this);
            QLabel      *text      = new QLabel("Rotate Degree   ", this);
            QPushButton *rm_button = new QPushButton("-", this);

            text->setAlignment(Qt::AlignCenter);

            edit_validator_ = std::make_unique<QDoubleValidator>();

            deg_edit_ = new QLineEdit(this);
            deg_edit_->setText(QString::number(deg));
            deg_edit_->setAlignment(Qt::AlignCenter);
            deg_edit_->setValidator(edit_validator_.get());

            layout->addWidget(rm_button);
            layout->addWidget(text);
            layout->addWidget(deg_edit_);

            connect(rm_button, &QPushButton::clicked, [=]
            {
                emit remove_this();
            });

            connect(deg_edit_, &QLineEdit::returnPressed, [=]
            {
                emit change_transform();
            });

            setContentsMargins(0, 0, 0, 0);
            layout->setContentsMargins(0, 0, 0, 0);
        }

        UnitType get_type() const noexcept override
        {
            return UnitType::Rotate;
        }

        tracer::Transform2 get_transform() const override
        {
            const real deg = deg_edit_->text().toFloat();
            const real rad = math::deg2rad(deg);
            return tracer::Transform2::rotate(rad);
        }

        Transform2DUnitWidget *clone() const override
        {
            const real deg = deg_edit_->text().toFloat();
            return new Rotate2DWidget(nullptr, deg);
        }

        void save_asset(AssetSaver &saver) const override
        {
            saver.write_string(deg_edit_->text());
        }

        void load_asset(AssetLoader &loader) override
        {
            deg_edit_->setText(loader.read_string());
        }

    private:

        QLineEdit *deg_edit_;

        std::unique_ptr<QValidator> edit_validator_;
    };

    class Scale2DWidget : public Transform2DUnitWidget
    {
    public:

        explicit Scale2DWidget(QWidget *parent, const Vec2 &ratio = { 1, 1 })
            : Transform2DUnitWidget(parent)
        {
            QHBoxLayout *layout    = new QHBoxLayout(this);
            QLabel      *text      = new QLabel("Scale Ratio     ", this);
            QPushButton *rm_button = new QPushButton("-", this);

            text->setAlignment(Qt::AlignCenter);

            edit_validator_ = std::make_unique<Vec2Validator>();

            ratio_edit_ = new QLineEdit(this);
            ratio_edit_->setText(QString("%1 %2").arg(ratio.x).arg(ratio.y));
            ratio_edit_->setAlignment(Qt::AlignCenter);
            ratio_edit_->setValidator(edit_validator_.get());

            layout->addWidget(rm_button);
            layout->addWidget(text);
            layout->addWidget(ratio_edit_);

            connect(rm_button, &QPushButton::clicked, [=]
            {
                emit remove_this();
            });

            connect(ratio_edit_, &QLineEdit::returnPressed, [=]
            {
                emit change_transform();
            });

            setContentsMargins(0, 0, 0, 0);
            layout->setContentsMargins(0, 0, 0, 0);
        }

        UnitType get_type() const noexcept override
        {
            return UnitType::Scale;
        }

        tracer::Transform2 get_transform() const override
        {
            real x, y;
            QString line = ratio_edit_->text();
            QTextStream(&line) >> x >> y;
            return tracer::Transform2::scale(x, y);
        }

        Transform2DUnitWidget *clone() const override
        {
            real x, y;
            QString line = ratio_edit_->text();
            QTextStream(&line) >> x >> y;
            return new Scale2DWidget(nullptr, { x, y });
        }

        void save_asset(AssetSaver &saver) const override
        {
            saver.write_string(ratio_edit_->text());
        }

        void load_asset(AssetLoader &loader) override
        {
            ratio_edit_->setText(loader.read_string());
        }

    private:

        QLineEdit *ratio_edit_;

        std::unique_ptr<QValidator> edit_validator_;
    };
}

Transform2DWidget::Transform2DWidget()
    : Transform2DWidget(std::vector<Transform2DUnitWidget*>{})
{

}

tracer::Transform2 Transform2DWidget::get_transform() const
{
    tracer::Transform2 ret;
    for(auto w : units_)
        ret *= w->get_transform();
    return ret;
}

Transform2DWidget *Transform2DWidget::clone() const
{
    std::vector<Transform2DUnitWidget*> units;
    for(auto u : units_)
        units.push_back(u->clone());
    return new Transform2DWidget(std::move(units));
}

void Transform2DWidget::save_asset(AssetSaver &saver) const
{
    saver.write(uint32_t(units_.size()));
    for(auto &u : units_)
    {
        saver.write(u->get_type());
        u->save_asset(saver);
    }
}

void Transform2DWidget::load_asset(AssetLoader &loader)
{
    for(auto &u : units_)
        delete u;
    units_.clear();

    const uint32_t unit_count = loader.read<uint32_t>();
    for(uint32_t i = 0; i < unit_count; ++i)
    {
        const auto type = loader.read<Transform2DUnitWidget::UnitType>();
        auto widget = create_new_unit_widget(type);
        push_back(widget);

        widget->load_asset(loader);
    }
}

Transform2DUnitWidget::UnitType Transform2DWidget::get_unit_type()
{
    const QStringList item_list = { "Translate", "Rotate", "Scale" };
    const QString item = QInputDialog::getItem(this, "Type", "Select transform type", item_list);
    if(item == "Translate")
        return Transform2DUnitWidget::UnitType::Translate;
    if(item == "Rotate")
        return Transform2DUnitWidget::UnitType::Rotate;
    return Transform2DUnitWidget::UnitType::Scale;
}

Transform2DUnitWidget *Transform2DWidget::create_new_unit_widget(Transform2DUnitWidget::UnitType type)
{
    if(type == Transform2DUnitWidget::UnitType::Translate)
        return new Translate2DWidget(this);
    if(type == Transform2DUnitWidget::UnitType::Rotate)
        return new Rotate2DWidget(this);
    return new Scale2DWidget(this);
}

Transform2DWidget::Transform2DWidget(std::vector<Transform2DUnitWidget*> units)
{
    units_ = std::move(units);

    add_up_  = new QPushButton("+", this);
    add_down_= new QPushButton("+", this);

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(add_up_);
    for(auto u : units_)
        layout_->addWidget(u);
    layout_->addWidget(add_down_);

    add_up_->hide();

    connect(add_up_, &QPushButton::clicked, [=] { add_up(); });
    connect(add_down_, &QPushButton::clicked, [=] { add_down(); });

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);
}

void Transform2DWidget::add_up()
{
    const Transform2DUnitWidget::UnitType unit_type = get_unit_type();
    Transform2DUnitWidget *unit_widget = create_new_unit_widget(unit_type);

    units_.insert(units_.begin(), unit_widget);
    layout_->insertWidget(1, unit_widget);

    if(units_.size() == 0)
        add_up_->hide();
    else
        add_up_->show();

    connect(unit_widget, &Transform2DUnitWidget::change_transform, [=]
    {
        emit change_transform();
    });

    connect(unit_widget, &Transform2DUnitWidget::remove_this, [=]
    {
        int index = -1;
        for(size_t i = 0; i < units_.size(); ++i)
        {
            if(units_[i] == unit_widget)
            {
                index = static_cast<int>(i);
                break;
            }
        }
        assert(index >= 0);

        delete units_[index];
        units_.erase(units_.begin() + index);

        if(units_.size() == 0)
            add_up_->hide();
        else
            add_up_->show();

        emit change_transform();
    });
}

void Transform2DWidget::add_down()
{
    const Transform2DUnitWidget::UnitType unit_type = get_unit_type();
    Transform2DUnitWidget *unit_widget = create_new_unit_widget(unit_type);
    push_back(unit_widget);
}

void Transform2DWidget::push_back(Transform2DUnitWidget *unit_widget)
{
    units_.push_back(unit_widget);
    layout_->insertWidget(layout_->count() - 1, unit_widget);

    if(units_.size() == 0)
        add_up_->hide();
    else
        add_up_->show();

    connect(unit_widget, &Transform2DUnitWidget::change_transform, [=]
    {
        emit change_transform();
    });

    connect(unit_widget, &Transform2DUnitWidget::remove_this, [=]
    {
        int index = -1;
        for(size_t i = 0; i < units_.size(); ++i)
        {
            if(units_[i] == unit_widget)
            {
                index = static_cast<int>(i);
                break;
            }
        }
        assert(index >= 0);

        delete units_[index];
        units_.erase(units_.begin() + index);

        if(units_.size() == 0)
            add_up_->hide();
        else
            add_up_->show();

        emit change_transform();
    });
}

AGZ_EDITOR_END
