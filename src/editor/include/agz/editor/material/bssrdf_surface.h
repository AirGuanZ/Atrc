#pragma once

#include <functional>

#include <QCheckBox>
#include <QVBoxLayout>
#include <QWidget>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/texture2d/range.h>
#include <agz/editor/texture2d/texture2d.h>
#include <agz/editor/ui/utility/collapsible.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;
class JSONExportContext;

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
class BSSRDFSurfaceWidget : public QWidget
{
public:

    struct InitData
    {
        bool use_bssrdf = false;

        bool own_A   = false;
        bool own_eta = false;

        Texture2DSlot *A    = nullptr;
        Texture2DSlot *dmfp = nullptr;
        Texture2DSlot *eta  = nullptr;
    };

    explicit BSSRDFSurfaceWidget(const InitData &init_data, ObjectContext &ctx);

    void set_dirty_callback(std::function<void()> callback);

    RC<tracer::BSSRDFSurface> create_tracer_object(
        RC<tracer::Texture2D> default_A,
        RC<tracer::Texture2D> default_eta) const;

    BSSRDFSurfaceWidget *clone();

    void to_config(tracer::ConfigGroup &mat_grp, JSONExportContext &ctx) const;

    void save_asset(AssetSaver &saver) const;
    
    void load_asset(AssetLoader &loader);

private:

    void update_disable_state();

    void call_dirty_callback();

    ObjectContext &ctx_;

    QCheckBox *enable_bssrdf_ = nullptr;

    std::function<void()> dirty_callback_;

    QCheckBox *own_A_   = nullptr;
    QCheckBox *own_eta_ = nullptr;

    Texture2DSlot *A_    = nullptr;
    Texture2DSlot *dmfp_ = nullptr;
    Texture2DSlot *eta_  = nullptr;
};

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::BSSRDFSurfaceWidget(
    const InitData &init_data, ObjectContext &ctx)
    : ctx_(ctx)
{
    enable_bssrdf_ = new QCheckBox("Enable BSSRDF", this);
    enable_bssrdf_->setChecked(init_data.use_bssrdf);

    if constexpr(!MUST_HAVE_A)
    {
        own_A_ = new QCheckBox("Don't use default A", this);
        own_A_->setChecked(init_data.own_A);
    }

    if constexpr(!MUST_HAVE_ETA)
    {
        own_eta_ = new QCheckBox("Don't use default eta", this);
        own_eta_->setChecked(init_data.own_eta);
    }

    A_ = init_data.A;
    if(!A_)
        A_ = new Texture2DSlot(ctx_, "Constant");
    
    dmfp_ = init_data.dmfp;
    if(!dmfp_)
    {
        auto range = new RangeWidget({ 0, real(0.5), real(0.05) });
        dmfp_ = new Texture2DSlot(ctx_, "Range", range, "Range");
    }

    eta_ = init_data.eta;
    if(!eta_)
    {
        auto range = new RangeWidget({ real(1.01), 3, real(1.5) });
        eta_ = new Texture2DSlot(ctx_, "Range", range, "Range");
    }

    update_disable_state();

    A_->set_dirty_callback   ([=] { call_dirty_callback(); });
    dmfp_->set_dirty_callback([=] { call_dirty_callback(); });
    eta_->set_dirty_callback ([=] { call_dirty_callback(); });

    connect(enable_bssrdf_, &QCheckBox::stateChanged, [=](int)
    {
        update_disable_state();
        call_dirty_callback();
    });

    if(own_A_)
    {
        connect(own_A_, &QCheckBox::stateChanged, [=](int)
        {
            update_disable_state();
            call_dirty_callback();
        });
    }

    if(own_eta_)
    {
        connect(own_eta_, &QCheckBox::stateChanged, [=](int)
        {
            update_disable_state();
            call_dirty_callback();
        });
    }

    Collapsible *A_sec = new Collapsible(this, "A");
    if(own_A_)
    {
        auto sec_widget = new QWidget(A_sec);
        auto sec_layout = new QVBoxLayout(sec_widget);

        sec_layout->addWidget(own_A_);
        sec_layout->addWidget(A_);

        A_sec->set_content_widget(sec_widget);
    }
    else
        A_sec->set_content_widget(A_);

    Collapsible *dmfp_sec = new Collapsible(this, "dmfp");
    dmfp_sec->set_content_widget(dmfp_);

    Collapsible *eta_sec = new Collapsible(this, "eta");
    if(own_eta_)
    {
        auto sec_widget = new QWidget(eta_sec);
        auto sec_layout = new QVBoxLayout(sec_widget);

        sec_layout->addWidget(own_eta_);
        sec_layout->addWidget(eta_);

        eta_sec->set_content_widget(sec_widget);
    }
    else
        eta_sec->set_content_widget(eta_);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(enable_bssrdf_);
    layout->addWidget(A_sec);
    layout->addWidget(dmfp_sec);
    layout->addWidget(eta_sec);
}

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
void BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::set_dirty_callback(
    std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
RC<tracer::BSSRDFSurface>
    BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::create_tracer_object(
        RC<tracer::Texture2D> default_A,
        RC<tracer::Texture2D> default_eta) const
{
    if(!enable_bssrdf_->isChecked())
        return newRC<tracer::BSSRDFSurface>();

    assert(MUST_HAVE_A   || default_A);
    assert(MUST_HAVE_ETA || default_eta);

    auto A = default_A;
    if(MUST_HAVE_A || own_A_->isChecked())
        A = A_->get_tracer_object();

    auto dmfp = dmfp_->get_tracer_object();

    auto eta = default_eta;
    if(MUST_HAVE_ETA || own_eta_->isChecked())
        eta = eta_->get_tracer_object();

    return create_normalized_diffusion_bssrdf_surface(
        std::move(A), std::move(dmfp), std::move(eta));
}

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA> *
    BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::clone()
{
    InitData init_data;

    init_data.use_bssrdf = enable_bssrdf_->isChecked();

    if(own_A_)
        init_data.own_A = own_A_->isChecked();
    if(own_eta_)
        init_data.own_eta = own_eta_->isChecked();

    init_data.A    = A_   ->clone();
    init_data.dmfp = dmfp_->clone();
    init_data.eta  = eta_ ->clone();

    return new BSSRDFSurfaceWidget(init_data, ctx_);
}

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
void BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::to_config(
    tracer::ConfigGroup &mat_grp, JSONExportContext &ctx) const
{
    if(!enable_bssrdf_->isChecked())
        return;

    if(MUST_HAVE_A || own_A_->isChecked())
        mat_grp.insert_child("bssrdf_A", A_->to_config(ctx));

    if(MUST_HAVE_ETA || own_eta_->isChecked())
        mat_grp.insert_child("bssrdf_eta", eta_->to_config(ctx));

    mat_grp.insert_child("bssrdf_dmfp", dmfp_->to_config(ctx));
}

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
void BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::save_asset(
    AssetSaver &saver) const
{
    saver.write(int32_t(enable_bssrdf_->isChecked() ? 1 : 0));
    if(own_A_)
        saver.write(int32_t(own_A_->isChecked() ? 1 : 0));
    if(own_eta_)
        saver.write(int32_t(own_eta_->isChecked() ? 1 : 0));
    A_->save_asset(saver);
    dmfp_->save_asset(saver);
    eta_->save_asset(saver);
}

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
void BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::load_asset(
    AssetLoader &loader)
{
    enable_bssrdf_->setChecked(loader.read<int32_t>() != 0);
    if(own_A_)
        own_A_->setChecked(loader.read<int32_t>() != 0);
    if(own_eta_)
        own_eta_->setChecked(loader.read<int32_t>() != 0);
    A_->load_asset(loader);
    dmfp_->load_asset(loader);
    eta_->load_asset(loader);
}

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
void BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::update_disable_state()
{
    if(!enable_bssrdf_->isChecked())
    {
        if(own_A_)
            own_A_->setDisabled(true);
        if(own_eta_)
            own_eta_->setDisabled(true);

        A_   ->setDisabled(true);
        dmfp_->setDisabled(true);
        eta_ ->setDisabled(true);

        return;
    }

    if(own_A_)
        own_A_->setDisabled(false);
    if(own_eta_)
        own_eta_->setDisabled(false);

    A_  ->setDisabled(!MUST_HAVE_A   && !own_A_->isChecked());
    eta_->setDisabled(!MUST_HAVE_ETA && !own_eta_->isChecked());

    dmfp_->setDisabled(false);
}

template<bool MUST_HAVE_A, bool MUST_HAVE_ETA>
void BSSRDFSurfaceWidget<MUST_HAVE_A, MUST_HAVE_ETA>::call_dirty_callback()
{
    if(dirty_callback_)
        dirty_callback_();
}

AGZ_EDITOR_END
