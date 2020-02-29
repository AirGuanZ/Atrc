#pragma once

#include <QLineEdit>
#include <QWidget>

#include <agz/editor/ui/utility/validator.h>

AGZ_EDITOR_BEGIN

class RealInput : public QWidget
{
    Q_OBJECT

public:

    explicit RealInput(QWidget *parent = nullptr);

    void set_value(real value);

    void set_alignment(Qt::Alignment alignment);

    real get_value() const;

signals:

    void edit_value(real new_value);

private:

    QLineEdit *edit_ = nullptr;
    std::unique_ptr<QDoubleValidator> validator_;
};

class Vec2Input : public QWidget
{
    Q_OBJECT

public:

    explicit Vec2Input(QWidget *parent = nullptr);

    void set_value(const Vec2 &value);

    Vec2 get_value() const;

signals:

    void edit_value(const Vec2 &new_value);

private:

    QLineEdit *edit_ = nullptr;
    std::unique_ptr<Vec2Validator> validator_;
};

class Vec3Input : public QWidget
{
    Q_OBJECT

public:

    explicit Vec3Input(QWidget *parent = nullptr);

    void set_value(const Vec3 &value);

    Vec3 get_value() const;

    void set_alignment(Qt::Alignment alignment);

signals:

    void edit_value(const Vec3 &new_value);

private:

    QLineEdit *edit_ = nullptr;
    std::unique_ptr<Vec3Validator> validator_;
};

class SpectrumInput : public QWidget
{
    Q_OBJECT

public:

    explicit SpectrumInput(QWidget *parent = nullptr);

    void set_value(const Spectrum &value);

    Spectrum get_value() const;

    void set_alignment(Qt::Alignment alignment);

signals:

    void edit_value(const Spectrum &new_value);

private:

    QLineEdit *edit_ = nullptr;
    std::unique_ptr<Vec3Validator> validator_;
};

AGZ_EDITOR_END
