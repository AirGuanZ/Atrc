#include <QLabel>
#include <QLineEdit>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/texture2d/texture2d_common_params.h>

AGZ_EDITOR_BEGIN

Texture2DCommonParamsWidget::Texture2DCommonParamsWidget(const InitData &init_data)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget     *inv_gamma_widget = new QWidget(this);
    QHBoxLayout *inv_gamma_layout = new QHBoxLayout(inv_gamma_widget);

    apply_inv_gamma_ = new QCheckBox("Inverse Gamma Correction", inv_gamma_widget);
    inv_gamma_       = new QLineEdit(inv_gamma_widget);

    inv_gamma_validator_ = std::make_unique<QDoubleValidator>();

    QWidget     *uv_widget = new QWidget(this);
    QHBoxLayout *uv_layout = new QHBoxLayout(uv_widget);

    inv_u_   = new QCheckBox("Inverse U", this);
    inv_v_   = new QCheckBox("Inverse V", this);
    swap_uv_ = new QCheckBox("Swap UV", this);

    transform_ = init_data.transform;
    if(!transform_)
        transform_ = new Transform2DWidget;

    QWidget *wrap_u_widget = new QWidget(this);
    QWidget *wrap_v_widget = new QWidget(this);
    QHBoxLayout *wrap_u_layout = new QHBoxLayout(wrap_u_widget);
    QHBoxLayout *wrap_v_layout = new QHBoxLayout(wrap_v_widget);
    QLabel *wrap_u_text = new QLabel("Wrap U", wrap_u_widget);
    QLabel *wrap_v_text = new QLabel("Wrap V", wrap_v_widget);

    wrap_u_ = new QComboBox(this);
    wrap_v_ = new QComboBox(this);

    apply_inv_gamma_->setChecked(init_data.apply_inv_gamma);
    inv_gamma_->setText(QString::number(init_data.inv_gamma));
    inv_gamma_->setAlignment(Qt::AlignCenter);
    inv_gamma_->setValidator(inv_gamma_validator_.get());
    if(!init_data.apply_inv_gamma)
        inv_gamma_->setDisabled(true);

    inv_u_  ->setChecked(init_data.inv_u);
    inv_v_  ->setChecked(init_data.inv_v);
    swap_uv_->setChecked(init_data.swap_uv);

    wrap_u_text->setAlignment(Qt::AlignCenter);
    wrap_v_text->setAlignment(Qt::AlignCenter);
    wrap_u_text->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    wrap_v_text->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    wrap_u_layout->addWidget(wrap_u_text);
    wrap_u_layout->addWidget(wrap_u_);
    wrap_v_layout->addWidget(wrap_v_text);
    wrap_v_layout->addWidget(wrap_v_);

    wrap_u_widget->setContentsMargins(0, 0, 0, 0);
    wrap_u_layout->setContentsMargins(0, 0, 0, 0);

    wrap_v_widget->setContentsMargins(0, 0, 0, 0);
    wrap_v_layout->setContentsMargins(0, 0, 0, 0);

    wrap_u_->addItems({ "Clamp", "Repeat", "Mirror" });
    wrap_v_->addItems({ "Clamp", "Repeat", "Mirror" });
    wrap_u_->setCurrentText(init_data.wrap_u);
    wrap_v_->setCurrentText(init_data.wrap_v);

    inv_gamma_widget->setContentsMargins(0, 0, 0, 0);
    inv_gamma_layout->setContentsMargins(0, 0, 0, 0);
    inv_gamma_layout->addWidget(apply_inv_gamma_);
    inv_gamma_layout->addWidget(inv_gamma_);

    uv_widget->setContentsMargins(0, 0, 0, 0);
    uv_layout->setContentsMargins(0, 0, 0, 0);
    uv_layout->addWidget(inv_u_);
    uv_layout->addWidget(inv_v_);
    uv_layout->addWidget(swap_uv_);

    layout->addWidget(inv_gamma_widget);
    layout->addWidget(uv_widget);
    layout->addWidget(wrap_u_widget);
    layout->addWidget(wrap_v_widget);
    layout->addWidget(transform_);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(apply_inv_gamma_, &QCheckBox::stateChanged,  [=](int)
    {
        inv_gamma_->setDisabled(!apply_inv_gamma_->isChecked());
        emit change_params();
    });

    connect(inv_gamma_, &QLineEdit::returnPressed, [=] { emit change_params(); });

    connect(inv_u_,   &QCheckBox::stateChanged, [=](int) { emit change_params(); });
    connect(inv_v_,   &QCheckBox::stateChanged, [=](int) { emit change_params(); });
    connect(swap_uv_, &QCheckBox::stateChanged, [=](int) { emit change_params(); });

    connect(transform_, &Transform2DWidget::change_transform, [=] { emit change_params(); });

    connect(wrap_u_, &QComboBox::currentTextChanged, [=](const QString &) { emit change_params(); });
    connect(wrap_v_, &QComboBox::currentTextChanged, [=](const QString &) { emit change_params(); });
}

Texture2DCommonParamsWidget *Texture2DCommonParamsWidget::clone() const
{
    InitData init_data;
    init_data.apply_inv_gamma = apply_inv_gamma_->isChecked();
    init_data.inv_gamma       = inv_gamma_->text().toFloat();
    init_data.inv_u           = inv_u_->isChecked();
    init_data.inv_v           = inv_v_->isChecked();
    init_data.swap_uv         = swap_uv_->isChecked();
    init_data.transform       = transform_->clone();
    init_data.wrap_u          = wrap_u_->currentText();
    init_data.wrap_v          = wrap_v_->currentText();
    return new Texture2DCommonParamsWidget(init_data);
}

tracer::Texture2DCommonParams Texture2DCommonParamsWidget::get_tracer_params() const
{
    tracer::Texture2DCommonParams ret;
    if(apply_inv_gamma_->isChecked())
        ret.inv_gamma = inv_gamma_->text().toFloat();
    ret.inv_u     = inv_u_->isChecked();
    ret.inv_v     = inv_v_->isChecked();
    ret.swap_uv   = swap_uv_->isChecked();
    ret.transform = transform_->get_transform();
    ret.wrap_u    = wrap_u_->currentText().toLower().toStdString();
    ret.wrap_v    = wrap_v_->currentText().toLower().toStdString();
    return ret;
}

void Texture2DCommonParamsWidget::save_asset(AssetSaver &saver) const
{
    saver.write(uint8_t(apply_inv_gamma_->isChecked() ? 1 : 0));
    saver.write(inv_gamma_->text().toFloat());

    saver.write(uint8_t(inv_u_->isChecked() ? 1 : 0));
    saver.write(uint8_t(inv_v_->isChecked() ? 1 : 0));
    saver.write(uint8_t(swap_uv_->isChecked() ? 1 : 0));

    transform_->save_asset(saver);

    saver.write_string(wrap_u_->currentText());
    saver.write_string(wrap_v_->currentText());
}

void Texture2DCommonParamsWidget::load_asset(AssetLoader &loader)
{
    apply_inv_gamma_->setChecked(loader.read<uint8_t>() != 0);
    inv_gamma_->setText(QString::number(loader.read<float>()));

    inv_u_->setChecked(loader.read<uint8_t>() != 0);
    inv_v_->setChecked(loader.read<uint8_t>() != 0);
    swap_uv_->setChecked(loader.read<uint8_t>() != 0);

    transform_->load_asset(loader);

    wrap_u_->setCurrentText(loader.read_string());
    wrap_v_->setCurrentText(loader.read_string());
}

AGZ_EDITOR_END
