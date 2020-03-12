#include <QVBoxLayout>

#include <agz/editor/ui/utility/vec_input.h>

AGZ_EDITOR_BEGIN

RealInput::RealInput(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    edit_ = new QLineEdit(this);
    validator_ = newBox<QDoubleValidator>();

    layout->addWidget(edit_);
    edit_->setText("0");
    edit_->setValidator(validator_.get());

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(edit_, &QLineEdit::returnPressed, [=]
    {
        emit edit_value(real(edit_->text().toDouble()));
    });
}

void RealInput::set_value(real value)
{
    edit_->setText(QString::number(value));
}

void RealInput::set_alignment(Qt::Alignment alignment)
{
    edit_->setAlignment(alignment);
}

real RealInput::get_value() const
{
    return real(edit_->text().toDouble());
}

Vec2Input::Vec2Input(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    edit_      = new QLineEdit(this);
    validator_ = newBox<Vec2Validator>();

    layout->addWidget(edit_);
    edit_->setText("0 0");
    edit_->setValidator(validator_.get());

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(edit_, &QLineEdit::returnPressed, [=]
    {
        QString text = edit_->text();
        const Vec2 new_value = Vec2Validator::parse(text);
        emit edit_value(new_value);
    });
}

void Vec2Input::set_value(const Vec2 &value)
{
    edit_->setText(QString("%1 %2").arg(value.x).arg(value.y));
}

Vec2 Vec2Input::get_value() const
{
    QString text = edit_->text();
    return Vec2Validator::parse(text);
}

Vec3Input::Vec3Input(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    edit_      = new QLineEdit(this);
    validator_ = newBox<Vec3Validator>();

    layout->addWidget(edit_);
    edit_->setText("0 0 0");
    edit_->setValidator(validator_.get());

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(edit_, &QLineEdit::returnPressed, [=]
    {
        QString text = edit_->text();
        const Vec3 new_value = Vec3Validator::parse(text);
        emit edit_value(new_value);
    });
}

void Vec3Input::set_value(const Vec3 &value)
{
    edit_->setText(QString("%1 %2 %3").arg(value.x).arg(value.y).arg(value.z));
}

Vec3 Vec3Input::get_value() const
{
    QString text = edit_->text();
    return Vec3Validator::parse(text);
}

void Vec3Input::set_alignment(Qt::Alignment alignment)
{
    edit_->setAlignment(alignment);
}

SpectrumInput::SpectrumInput(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    edit_ = new QLineEdit(this);
    validator_ = newBox<Vec3Validator>();

    layout->addWidget(edit_);
    edit_->setText("0 0 0");
    edit_->setValidator(validator_.get());

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(edit_, &QLineEdit::returnPressed, [=]
    {
        QString text = edit_->text();
        const Vec3 new_value = Vec3Validator::parse(text);
        emit edit_value({ new_value.x, new_value.y, new_value.z });
    });
}

void SpectrumInput::set_value(const Spectrum &value)
{
    edit_->setText(QString("%1 %2 %3").arg(value.r).arg(value.g).arg(value.b));
}

Spectrum SpectrumInput::get_value() const
{
    QString text = edit_->text();
    const Vec3 v = Vec3Validator::parse(text);
    return { v.x, v.y, v.z };
}

void SpectrumInput::set_alignment(Qt::Alignment alignment)
{
    edit_->setAlignment(alignment);
}

AGZ_EDITOR_END
