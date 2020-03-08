#include <QGridLayout>
#include <QLabel>
#include <QInputDialog>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/ui/transform3d_seq_widget.h>

AGZ_EDITOR_BEGIN

namespace
{
    class Translate3DSeqUnitWidget : public Transform3DSeqUnitWidget
    {
    public:

        explicit Translate3DSeqUnitWidget(QWidget *parent, const Vec3 &offset = {})
            : Transform3DSeqUnitWidget(parent)
        {
            QHBoxLayout *layout = new QHBoxLayout(this);
            QLabel *text = new QLabel("Translate Offset", this);
            QPushButton *rm_button = new QPushButton("-", this);

            text->setAlignment(Qt::AlignCenter);

            edit_validator_ = std::make_unique<Vec3Validator>();

            offset_edit_ = new QLineEdit(this);
            offset_edit_->setText(QString("%1 %2 %3").arg(offset.x).arg(offset.y).arg(offset.z));
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

        tracer::Transform3 get_transform() const override
        {
            real x, y, z;
            QString line = offset_edit_->text();
            QTextStream(&line) >> x >> y >> z;
            return tracer::Transform3::translate(x, y, z);
        }

        Translate3DSeqUnitWidget *clone() const override
        {
            real x, y, z;
            QString line = offset_edit_->text();
            QTextStream(&line) >> x >> y >> z;
            return new Translate3DSeqUnitWidget(nullptr, { x, y, z });
        }

        void save_asset(AssetSaver &saver) const override
        {
            saver.write_string(offset_edit_->text());
        }

        void load_asset(AssetLoader &loader) override
        {
            offset_edit_->setText(loader.read_string());
        }

        std::shared_ptr<tracer::ConfigGroup> to_config() const override
        {
            real x, y, z;
            QString line = offset_edit_->text();
            QTextStream(&line) >> x >> y >> z;

            auto grp = std::make_shared<tracer::ConfigGroup>();
            grp->insert_str("type", "translate");
            grp->insert_child("offset", tracer::ConfigArray::from_vec3({ x, y, z }));
            return grp;
        }

    private:

        QLineEdit *offset_edit_;

        std::unique_ptr<Vec3Validator> edit_validator_;
    };

    class Rotate3DSeqUnitWidget : public Transform3DSeqUnitWidget
    {
    public:

        explicit Rotate3DSeqUnitWidget(QWidget *parent, const Vec3 &axis = { 1, 0, 0 }, real deg = 0)
            : Transform3DSeqUnitWidget(parent)
        {
            QHBoxLayout *layout = new QHBoxLayout(this);
            QPushButton *rm_button = new QPushButton("-", this);

            edit_validator_ = std::make_unique<QDoubleValidator>();

            deg_edit_ = new QLineEdit(this);
            deg_edit_->setText(QString::number(deg));
            deg_edit_->setAlignment(Qt::AlignCenter);
            deg_edit_->setValidator(edit_validator_.get());

            axis_ = new Vec3Input;
            axis_->set_alignment(Qt::AlignCenter);
            axis_->set_value({ 1, 0, 0 });

            layout->addWidget(rm_button);
            layout->addWidget(new QLabel("Axis"));
            layout->addWidget(axis_);
            layout->addWidget(new QLabel("Deg"));
            layout->addWidget(deg_edit_);

            connect(rm_button, &QPushButton::clicked, [=]
            {
                emit remove_this();
            });

            connect(deg_edit_, &QLineEdit::returnPressed, [=]
            {
                emit change_transform();
            });

            connect(axis_, &Vec3Input::edit_value, [=](const Vec3 &)
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

        tracer::Transform3 get_transform() const override
        {
            const real deg = deg_edit_->text().toFloat();
            const real rad = math::deg2rad(deg);
            const Vec3 axis = axis_->get_value();
            return tracer::Transform3::rotate(axis, rad);
        }

        Transform3DSeqUnitWidget *clone() const override
        {
            const real deg = deg_edit_->text().toFloat();
            const Vec3 axis = axis_->get_value();
            return new Rotate3DSeqUnitWidget(nullptr, axis, deg);
        }

        void save_asset(AssetSaver &saver) const override
        {
            saver.write_string(deg_edit_->text());
            saver.write(axis_->get_value());
        }

        void load_asset(AssetLoader &loader) override
        {
            deg_edit_->setText(loader.read_string());
            axis_->set_value(loader.read<Vec3>());
        }

        std::shared_ptr<tracer::ConfigGroup> to_config() const override
        {
            const real deg = deg_edit_->text().toFloat();
            const Vec3 axis = axis_->get_value();

            auto grp = std::make_shared<tracer::ConfigGroup>();
            grp->insert_str("type", "rotate");
            grp->insert_child("axis", tracer::ConfigArray::from_vec3(axis));
            grp->insert_real("deg", deg);
            return grp;
        }

    private:

        QLineEdit *deg_edit_;
        Vec3Input *axis_;

        std::unique_ptr<QValidator> edit_validator_;
    };

    template<int Axis>
    class RotateAxis3DSeqUnitWidget : public Transform3DSeqUnitWidget
    {
    public:

        explicit RotateAxis3DSeqUnitWidget(QWidget *parent, real deg = 0)
            : Transform3DSeqUnitWidget(parent)
        {
            QPushButton *rm = new QPushButton("-");

            QLabel *text;
            if constexpr(Axis == 0)
                text = new QLabel("Deg X");
            else if constexpr(Axis == 1)
                text = new QLabel("Deg Y");
            else
                text = new QLabel("Deg Z");

            deg_ = new RealInput;
            deg_->set_value(deg);
            deg_->set_alignment(Qt::AlignCenter);

            QHBoxLayout *layout = new QHBoxLayout(this);
            layout->addWidget(rm);
            layout->addWidget(text);
            layout->addWidget(deg_);

            setContentsMargins(0, 0, 0, 0);
            layout->setContentsMargins(0, 0, 0, 0);

            connect(rm, &QPushButton::clicked, [=]
            {
                emit remove_this();
            });

            connect(deg_, &RealInput::edit_value, [=](real)
            {
                emit change_transform();
            });
        }

        UnitType get_type() const noexcept override
        {
            if constexpr(Axis == 0)
                return UnitType::RotateX;
            if constexpr(Axis == 1)
                return UnitType::RotateY;
            return UnitType::RotateZ;
        }

        tracer::Transform3 get_transform() const override
        {
            const real rad = math::deg2rad(deg_->get_value());
            if constexpr(Axis == 0)
                return tracer::Transform3::rotate_x(rad);
            if constexpr(Axis == 1)
                return tracer::Transform3::rotate_y(rad);
            return tracer::Transform3::rotate_z(rad);
        }

        Transform3DSeqUnitWidget *clone() const override
        {
            return new RotateAxis3DSeqUnitWidget<Axis>(nullptr, deg_->get_value());
        }

        void save_asset(AssetSaver &saver) const override
        {
            saver.write(deg_->get_value());
        }

        void load_asset(AssetLoader &loader) override
        {
            deg_->set_value(loader.read<real>());
        }

        std::shared_ptr<tracer::ConfigGroup> to_config() const override
        {
            auto grp = std::make_shared<tracer::ConfigGroup>();

            if constexpr(Axis == 0)
                grp->insert_str("type", "rotate_x");
            else if constexpr(Axis == 1)
                grp->insert_str("type", "rotate_y");
            else
                grp->insert_str("type", "rotate_z");

            grp->insert_real("deg", deg_->get_value());

            return grp;
        }

    private:

        RealInput *deg_;
    };

    class Scale3DSeqUnitWidget : public Transform3DSeqUnitWidget
    {
    public:

        explicit Scale3DSeqUnitWidget(QWidget *parent, const Vec3 &ratio = { 1, 1, 1 })
            : Transform3DSeqUnitWidget(parent)
        {
            QHBoxLayout *layout = new QHBoxLayout(this);
            QLabel *text = new QLabel("Scale Ratio", this);
            QPushButton *rm_button = new QPushButton("-", this);

            text->setAlignment(Qt::AlignCenter);

            edit_validator_ = std::make_unique<Vec3Validator>();

            ratio_edit_ = new QLineEdit(this);
            ratio_edit_->setText(QString("%1 %2 %3").arg(ratio.x).arg(ratio.y).arg(ratio.z));
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

        tracer::Transform3 get_transform() const override
        {
            real x, y, z;
            QString line = ratio_edit_->text();
            QTextStream(&line) >> x >> y >> z;
            return tracer::Transform3::scale(x, y, z);
        }

        Transform3DSeqUnitWidget *clone() const override
        {
            real x, y, z;
            QString line = ratio_edit_->text();
            QTextStream(&line) >> x >> y >> z;
            return new Scale3DSeqUnitWidget(nullptr, { x, y, z });
        }

        void save_asset(AssetSaver &saver) const override
        {
            saver.write_string(ratio_edit_->text());
        }

        void load_asset(AssetLoader &loader) override
        {
            ratio_edit_->setText(loader.read_string());
        }

        std::shared_ptr<tracer::ConfigGroup> to_config() const override
        {
            real x, y, z;
            QString line = ratio_edit_->text();
            QTextStream(&line) >> x >> y >> z;

            auto grp = std::make_shared<tracer::ConfigGroup>();
            grp->insert_str("type", "scale");
            grp->insert_child("ratio", tracer::ConfigArray::from_vec3({ x, y, z }));
            return grp;
        }

    private:

        QLineEdit *ratio_edit_;

        std::unique_ptr<QValidator> edit_validator_;
    };
}

Transform3DSeqWidget::Transform3DSeqWidget()
    : Transform3DSeqWidget(std::vector<Transform3DSeqUnitWidget *>{})
{

}

tracer::Transform3 Transform3DSeqWidget::get_transform() const
{
    tracer::Transform3 ret;
    for(auto w : units_)
        ret *= w->get_transform();
    return ret;
}

Transform3DSeqWidget *Transform3DSeqWidget::clone() const
{
    std::vector<Transform3DSeqUnitWidget *> units;
    for(auto u : units_)
        units.push_back(u->clone());
    return new Transform3DSeqWidget(std::move(units));
}

void Transform3DSeqWidget::save_asset(AssetSaver &saver) const
{
    saver.write(uint32_t(units_.size()));
    for(auto &u : units_)
    {
        saver.write(u->get_type());
        u->save_asset(saver);
    }
}

void Transform3DSeqWidget::load_asset(AssetLoader &loader)
{
    for(auto &u : units_)
        delete u;
    units_.clear();

    const uint32_t unit_count = loader.read<uint32_t>();
    for(uint32_t i = 0; i < unit_count; ++i)
    {
        const auto type = loader.read<Transform3DSeqUnitWidget::UnitType>();
        auto widget = create_new_unit_widget(type);
        push_back(widget);

        widget->load_asset(loader);
    }
}

std::shared_ptr<tracer::ConfigArray> Transform3DSeqWidget::to_config() const
{
    auto arr = std::make_shared<tracer::ConfigArray>();
    for(auto &u : units_)
        arr->push_back(u->to_config());
    return arr;
}

Transform3DSeqUnitWidget::UnitType Transform3DSeqWidget::get_unit_type()
{
    const QStringList item_list = { "Translate", "RotateX", "RotateY", "RotateZ", "Rotate", "Scale" };
    const QString item = QInputDialog::getItem(this, "Type", "Select transform type", item_list);
    if(item == "Translate")
        return Transform3DSeqUnitWidget::UnitType::Translate;
    if(item == "RotateX")
        return Transform3DSeqUnitWidget::UnitType::RotateX;
    if(item == "RotateY")
        return Transform3DSeqUnitWidget::UnitType::RotateY;
    if(item == "RotateZ")
        return Transform3DSeqUnitWidget::UnitType::RotateZ;
    if(item == "Rotate")
        return Transform3DSeqUnitWidget::UnitType::Rotate;
    return Transform3DSeqUnitWidget::UnitType::Scale;
}

Transform3DSeqUnitWidget *Transform3DSeqWidget::create_new_unit_widget(Transform3DSeqUnitWidget::UnitType type)
{
    if(type == Transform3DSeqUnitWidget::UnitType::Translate)
        return new Translate3DSeqUnitWidget(this);
    if(type == Transform3DSeqUnitWidget::UnitType::Rotate)
        return new Rotate3DSeqUnitWidget(this);
    if(type == Translate3DSeqUnitWidget::UnitType::RotateX)
        return new RotateAxis3DSeqUnitWidget<0>(this);
    if(type == Translate3DSeqUnitWidget::UnitType::RotateY)
        return new RotateAxis3DSeqUnitWidget<1>(this);
    if(type == Translate3DSeqUnitWidget::UnitType::RotateZ)
        return new RotateAxis3DSeqUnitWidget<2>(this);
    return new Scale3DSeqUnitWidget(this);
}

Transform3DSeqWidget::Transform3DSeqWidget(std::vector<Transform3DSeqUnitWidget *> units)
{
    units_ = std::move(units);

    add_up_ = new QPushButton("+", this);
    add_down_ = new QPushButton("+", this);

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

void Transform3DSeqWidget::add_up()
{
    const Transform3DSeqUnitWidget::UnitType unit_type = get_unit_type();
    Transform3DSeqUnitWidget *unit_widget = create_new_unit_widget(unit_type);

    units_.insert(units_.begin(), unit_widget);
    layout_->insertWidget(1, unit_widget);

    if(units_.size() == 0)
        add_up_->hide();
    else
        add_up_->show();

    connect(unit_widget, &Transform3DSeqUnitWidget::change_transform, [=]
    {
        emit change_transform();
    });

    connect(unit_widget, &Transform3DSeqUnitWidget::remove_this, [=]
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

void Transform3DSeqWidget::add_down()
{
    const Transform3DSeqUnitWidget::UnitType unit_type = get_unit_type();
    Transform3DSeqUnitWidget *unit_widget = create_new_unit_widget(unit_type);
    push_back(unit_widget);
}

void Transform3DSeqWidget::push_back(Transform3DSeqUnitWidget *unit_widget)
{
    units_.push_back(unit_widget);
    layout_->insertWidget(layout_->count() - 1, unit_widget);

    if(units_.size() == 0)
        add_up_->hide();
    else
        add_up_->show();

    connect(unit_widget, &Transform3DSeqUnitWidget::change_transform, [=]
    {
        emit change_transform();
    });

    connect(unit_widget, &Transform3DSeqUnitWidget::remove_this, [=]
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
