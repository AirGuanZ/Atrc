#pragma once

#include <map>
#include <set>

#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <agz/editor/ui/utility/qsignal_to_callback.h>

AGZ_EDITOR_BEGIN

/*
Ownership:
    Other ResourceWidget
      ==> ResourceSlot
            ==> ResourceReference
                  --> ResourceInPool
                        ==>
            ==============> ResourcePanel
                              ==> ResourceWidget

Each object has a 'dirty' flag incidcating whether the corresponding tracer object need to be updated.
When a widget is edited, its dirty flag is set to true and propagated to all its ancestors, and the
first affected root object will emit a message to editor. The editor will then frozen all accesses to
the editor object DAG (including the rendering process), and call the updating method of each root object
to refresh the tracer object DAG. All objecs with 'true' dirty flag must update their corresponding tracer objects.
*/

template<typename TracerObject> class ResourceWidget;
template<typename TracerObject> class ResourceWidgetCreator;
template<typename TracerObject> class ResourceWidgetFactory;
template<typename TracerObject> class ResourcePanel;
template<typename TracerObject> class ResourceReference;
template<typename TracerObject> class ResourceInPool;
template<typename TracerObject> class ResourceSlot;
template<typename TracerObject> class ResourcePool;

class ResourceObjectException : public std::exception
{
public:

    explicit ResourceObjectException(const QString &msg);

    char const *what() const override;

    const QString &msg() const noexcept;

private:

    QString msg_;
};

/**
 * @brief widget for holding and editing a tracer object
 */
template<typename TracerObject>
class ResourceWidget : public QWidget
{
public:

    using QWidget::QWidget;

    /**
     * @brief set the callback function called when this widget is set to dirty
     *
     * called by owner of the widget instance in object DAG
     */
    void set_dirty_callback(std::function<void()> callback);

    /**
     * @brief get the newest tracer object
     */
    std::shared_ptr<TracerObject> update_tracer_object();

    /**
     * @brief clone the resource widget and tracer object
     */
    virtual ResourceWidget<TracerObject> *clone() = 0;

protected:

    /**
     * @brief set the dirty flag to true and call the dirty_callback function (if presents)
     */
    void set_dirty_flag();

    /**
     * @brief update the corresponding tracer object
     */
    virtual void update_tracer_object_impl() = 0;

    std::shared_ptr<TracerObject> tracer_object_;
    
private:

    /**
     * @brief check whether need to update the corresponding tracer object and set the dirty flag to false
     */
    bool check_update_flag() noexcept;

    bool dirty_flag_ = false;

    std::function<void()> dirty_callback_;
};

/**
 * @brief resource widget creator registered in factory
 */
template<typename TracerObject>
class ResourceWidgetCreator : public misc::uncopyable_t
{
public:

    virtual ~ResourceWidgetCreator() = default;

    virtual QString name() const = 0;

    virtual ResourceWidget<TracerObject> *create_widget() const = 0;
};

/**
 * @brief map type name to new resource widget
 */
template<typename TracerObject>
class ResourceWidgetFactory : public misc::uncopyable_t
{
public:

    using Widget  = ResourceWidget<TracerObject>;
    using Creator = ResourceWidgetCreator<TracerObject>;

    void add_creator(std::unique_ptr<Creator> creator);

    const QStringList &get_type_names() const noexcept;

    Widget *create_widget(QStringView name) const;

private:

    QStringList type_names_;

    std::map<QString, std::unique_ptr<Creator>, std::less<>> name2creator_;
};

/**
 * @brief type selector + resource widget
 */
template<typename TracerObject>
class ResourcePanel : public QWidget
{
public:

    explicit ResourcePanel(const ResourceWidgetFactory<TracerObject> &factory, QStringView default_type);

    void set_dirty_callback(std::function<void()> callback);

    std::shared_ptr<TracerObject> update_tracer_object();

    ResourcePanel<TracerObject> *clone() const;

private:

    ResourcePanel(const 
        ResourceWidgetFactory<TracerObject> &factory,
        ResourceWidget<TracerObject> *rsc_widget, QStringView current_type_name);

    const ResourceWidgetFactory<TracerObject> &factory_;

    QSignalToCallback signal_to_callback_;

    QVBoxLayout *layout_        = nullptr;
    QComboBox   *type_selector_ = nullptr;

    ResourceWidget<TracerObject> *rsc_widget_ = nullptr;

    std::function<void()> dirty_callback_;
};

/**
 * @brief reference to a resource instance in pool
 *
 * when the resource is destroyed, all its references's `removed_callback` is automatically called
 */
template<typename TracerObject>
class ResourceReference : public misc::uncopyable_t
{
public:

    explicit ResourceReference(ResourceInPool<TracerObject> *rsc_in_pool);

    ~ResourceReference();

    void set_dirty_callback(std::function<void()> callback);

    void set_removed_callback(std::function<void()> callback);

    std::shared_ptr<TracerObject> update_tracer_object();

    ResourceInPool<TracerObject> *get_resource();

private:

    friend class ResourceInPool<TracerObject>;

    void call_dirty_callback();

    void call_remove_callback();

    std::function<void()> dirty_callback_;
    std::function<void()> removed_callback_;

    ResourceInPool<TracerObject> *rsc_in_pool_;
};

/**
 * @brief resource instance in pool
 *
 * when an instance is destroyed, all its references's 'removed_callback' is automatically called.
 * After that, those references become invalid.
 */
template<typename TracerObject>
class ResourceInPool : public misc::uncopyable_t
{
public:

    explicit ResourceInPool(std::unique_ptr<ResourcePanel<TracerObject>> panel);

    ResourceInPool(const ResourceWidgetFactory<TracerObject> &factory, QStringView default_type);

    ~ResourceInPool();

    std::unique_ptr<ResourceReference<TracerObject>> create_reference();

    std::shared_ptr<TracerObject> update_tracer_object();

    ResourcePanel<TracerObject> *get_panel();

    ResourcePanel<TracerObject> *clone_panel();

private:

    friend class ResourceReference<TracerObject>;

    void remove_reference(ResourceReference<TracerObject> *ref);

    std::set<ResourceReference<TracerObject>*> references_;

    QPointer<ResourcePanel<TracerObject>> panel_;
};

/**
 * @brief resource pool interface
 */
template<typename TracerObject>
class ResourcePool : public misc::uncopyable_t
{
public:

    virtual ~ResourcePool() = default;

    virtual std::unique_ptr<ResourceReference<TracerObject>> select_resource() = 0;

    virtual ResourceInPool<TracerObject> *add_resource(
        QStringView name, std::unique_ptr<ResourcePanel<TracerObject>> panel) = 0;

    virtual bool is_valid_name(QStringView name) const = 0;

    virtual void show_edit_panel(ResourcePanel<TracerObject> *rsc) = 0;
};

/**
 * @brief single resource container
 *
 * 1. reference to a resource instance in pool
 *      Opr I.   edit resource
 *      Opr II.  break link
 *      Opr III. select in pool
 *      Opr IV.  create new
 * 2. resource panel
 *      Opr I.  select in pool
 *      Opr II. add to pool
 */
template<typename TracerObject>
class ResourceSlot : public QWidget, public misc::uncopyable_t
{
public:

    ResourceSlot(
        ResourcePool<TracerObject> &pool, const ResourceWidgetFactory<TracerObject> &factory, QStringView default_type);

    ~ResourceSlot();

    void set_dirty_callback(std::function<void()> callback);

    std::shared_ptr<TracerObject> update_tracer_object();

private:

    QSignalToCallback signal_to_callback_;

    ResourcePool<TracerObject> &pool_;
    const ResourceWidgetFactory<TracerObject> &factory_;
    QString default_panel_type_;

    std::function<void()> dirty_callback_;

    QVBoxLayout *layout_ = nullptr;

    // exactly one of `reference_` and `owned_panel_` is non-null

    std::unique_ptr<ResourceReference<TracerObject>> reference_;
    ResourcePanel<TracerObject>                     *owned_panel_ = nullptr;

    QWidget *reference_button_widget_ = nullptr;
    QWidget *owned_button_widget_     = nullptr;
};

inline ResourceObjectException::ResourceObjectException(const QString &msg)
    : msg_(msg)
{

}

inline char const *ResourceObjectException::what() const
{
    return msg_.toStdString().c_str();
}

inline const QString &ResourceObjectException::msg() const noexcept
{
    return msg_;
}

template<typename TracerObject>
void ResourceWidget<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourceWidget<TracerObject>::update_tracer_object()
{
    assert(tracer_object_);
    if(check_update_flag())
        update_tracer_object_impl();
    return tracer_object_;
}

template<typename TracerObject>
void ResourceWidget<TracerObject>::set_dirty_flag()
{
    dirty_flag_ = true;
    if(dirty_callback_)
        dirty_callback_();
}

template<typename TracerObject>
bool ResourceWidget<TracerObject>::check_update_flag() noexcept
{
    const bool ret = dirty_flag_;
    dirty_flag_ = false;
    return ret;
}

template<typename TracerObject>
void ResourceWidgetFactory<TracerObject>::add_creator(std::unique_ptr<Creator> creator)
{
    assert(creator);

    QString name = creator->name();
    if(name2creator_.find(name) != name2creator_.end())
        throw ResourceObjectException("repeated resource widget creator name: " + name);
    type_names_.push_back(name);

    name2creator_.insert(std::make_pair(std::move(name), std::move(creator)));
}

template<typename TracerObject>
const QStringList &ResourceWidgetFactory<TracerObject>::get_type_names() const noexcept
{
    return type_names_;
}

template<typename TracerObject>
typename ResourceWidgetFactory<TracerObject>::Widget*
    ResourceWidgetFactory<TracerObject>::create_widget(QStringView name) const
{
    const auto it = name2creator_.find(name);
    if(it == name2creator_.end())
        throw ResourceObjectException("unknown resource widget creator name: " + name.toString());
    return it->second->create_widget();
}

template<typename TracerObject>
ResourcePanel<TracerObject>::ResourcePanel(const ResourceWidgetFactory<TracerObject> &factory, QStringView default_type)
    : factory_(factory)
{
    type_selector_ = new QComboBox(this);
    type_selector_->addItems(factory_.get_type_names());
    type_selector_->setCurrentText(default_type.toString());

    auto change_type = [=](const QString &new_type)
    {
        if(rsc_widget_)
            delete rsc_widget_;

        rsc_widget_ = factory_.create_widget(new_type);
        rsc_widget_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });

        layout_->addWidget(rsc_widget_);
    };

    signal_to_callback_.connect_callback(type_selector_, &QComboBox::currentTextChanged, change_type);

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(type_selector_);

    change_type(default_type.toString());
}

template<typename TracerObject>
void ResourcePanel<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourcePanel<TracerObject>::update_tracer_object()
{
    return rsc_widget_->update_tracer_object();
}

template<typename TracerObject>
ResourcePanel<TracerObject> *ResourcePanel<TracerObject>::clone() const
{
    return new ResourcePanel(factory_, rsc_widget_->clone(), type_selector_->currentText());
}

template<typename TracerObject>
ResourcePanel<TracerObject>::ResourcePanel(
    const ResourceWidgetFactory<TracerObject> &factory,
    ResourceWidget<TracerObject> *rsc_widget, QStringView current_type_name)
    : factory_(factory)
{
    type_selector_ = new QComboBox(this);
    type_selector_->addItems(factory_.get_type_names());
    type_selector_->setCurrentText(current_type_name.toString());

    signal_to_callback_.connect_callback(
        type_selector_, &QComboBox::currentTextChanged, [=](const QString &new_type)
    {
        if(rsc_widget_)
            delete rsc_widget_;

        rsc_widget_ = factory_.create_widget(new_type);
        rsc_widget_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });

        layout_->addWidget(rsc_widget_);
    });

    rsc_widget_ = rsc_widget;

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(type_selector_);
    layout_->addWidget(rsc_widget_);
}

template<typename TracerObject>
ResourceReference<TracerObject>::ResourceReference(ResourceInPool<TracerObject> *rsc_in_pool)
    : rsc_in_pool_(rsc_in_pool)
{
    assert(rsc_in_pool);
}

template<typename TracerObject>
ResourceReference<TracerObject>::~ResourceReference()
{
    if(rsc_in_pool_)
        rsc_in_pool_->remove_reference(this);
}

template<typename TracerObject>
void ResourceReference<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
void ResourceReference<TracerObject>::set_removed_callback(std::function<void()> callback)
{
    removed_callback_ = std::move(callback);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourceReference<TracerObject>::update_tracer_object()
{
    assert(rsc_in_pool_);
    return rsc_in_pool_->update_tracer_object();
}

template<typename TracerObject>
ResourceInPool<TracerObject> *ResourceReference<TracerObject>::get_resource()
{
    assert(rsc_in_pool_);
    return rsc_in_pool_;
}

template<typename TracerObject>
void ResourceReference<TracerObject>::call_dirty_callback()
{
    if(dirty_callback_)
        dirty_callback_();
}

template<typename TracerObject>
void ResourceReference<TracerObject>::call_remove_callback()
{
    if(removed_callback_)
        removed_callback_();
}

template<typename TracerObject>
ResourceInPool<TracerObject>::ResourceInPool(std::unique_ptr<ResourcePanel<TracerObject>> panel)
{
    panel_ = panel.release();
    panel_->set_dirty_callback([=]
    {
        for(auto ref : references_)
            ref->call_dirty_callback();
    });
}

template<typename TracerObject>
ResourceInPool<TracerObject>::ResourceInPool(
    const ResourceWidgetFactory<TracerObject> &factory, QStringView default_type)
{
    panel_ = new ResourcePanel<TracerObject>(factory, default_type);
    panel_->set_dirty_callback([=]
    {
        for(auto ref : references_)
            ref->call_dirty_callback();
    });
}

template<typename TracerObject>
ResourceInPool<TracerObject>::~ResourceInPool()
{
    while(!references_.empty())
    {
        auto ref = *references_.begin();
        ref->call_remove_callback();
        ref->rsc_in_pool_ = nullptr;
        references_.erase(references_.begin());
    }

    ResourcePanel<TracerObject> *panel = panel_;
    delete panel;
}

template<typename TracerObject>
std::unique_ptr<ResourceReference<TracerObject>> ResourceInPool<TracerObject>::create_reference()
{
    auto ret = std::make_unique<ResourceReference<TracerObject>>(this);
    references_.insert(ret.get());
    return ret;
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::remove_reference(ResourceReference<TracerObject> *ref)
{
    ref->rsc_in_pool_ = nullptr;
    references_.erase(ref);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourceInPool<TracerObject>::update_tracer_object()
{
    return panel_->update_tracer_object();
}

template<typename TracerObject>
ResourcePanel<TracerObject> *ResourceInPool<TracerObject>::get_panel()
{
    return panel_;
}

template<typename TracerObject>
ResourcePanel<TracerObject> *ResourceInPool<TracerObject>::clone_panel()
{
    return panel_->clone();
}

template<typename TracerObject>
ResourceSlot<TracerObject>::ResourceSlot(
    ResourcePool<TracerObject> &pool, const ResourceWidgetFactory<TracerObject> &factory, QStringView default_type)
    : pool_(pool), factory_(factory), default_panel_type_(default_type.toString())
{
    owned_panel_ = new ResourcePanel<TracerObject>(factory_, default_type);
    owned_panel_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
    });

    reference_button_widget_ = new QWidget(this);
    owned_button_widget_     = new QWidget(this);

    {
        QVBoxLayout *reference_button_layout = new QVBoxLayout(reference_button_widget_);
        QVBoxLayout *owned_button_layout     = new QVBoxLayout(owned_button_widget_);
        
        QPushButton *create         = new QPushButton("Create", reference_button_widget_);
        QPushButton *select_in_pool = new QPushButton("Select", reference_button_widget_);
        QPushButton *edit_rsc       = new QPushButton("Edit",   reference_button_widget_);
        QPushButton *break_link     = new QPushButton("Unlink", reference_button_widget_);

        QPushButton *owned_select_in_pool = new QPushButton("Select",      owned_button_widget_);
        QPushButton *add_to_pool          = new QPushButton("Add to Pool", owned_button_widget_);

        // callback when the referenced resource is removed from pool
        // clone the referenced resource as my own panel and destroy the reference
        auto reference_removed_callback = [=]
        {
            owned_panel_ = reference_->get_resource()->clone_panel();
            owned_panel_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            layout_->addWidget(owned_panel_);

            reference_.reset();

            reference_button_widget_->hide();
            owned_button_widget_    ->show();

            if(dirty_callback_)
                dirty_callback_();
        };

        // delete the reference and create a new panel
        signal_to_callback_.connect_callback(create, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);

            reference_.reset();

            owned_panel_ = new ResourcePanel<TracerObject>(factory_, default_panel_type_);
            owned_panel_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            layout_->addWidget(owned_panel_);

            reference_button_widget_->hide();
            owned_button_widget_    ->show();

            if(dirty_callback_)
                dirty_callback_();
        });

        // delete current reference and select a new one from pool
        signal_to_callback_.connect_callback(select_in_pool, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);

            auto new_ref = pool_.select_resource();
            if(!new_ref)
                return;

            reference_ = std::move(new_ref);
            reference_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            reference_->set_removed_callback(reference_removed_callback);

            if(dirty_callback_)
                dirty_callback_();
        });

        // edit the referenced resource
        signal_to_callback_.connect_callback(edit_rsc, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);
            pool_.show_edit_panel(reference_->get_resource()->get_panel());
        });

        // clone the referenced resource as my own panel and destroy the reference
        signal_to_callback_.connect_callback(break_link, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);

            owned_panel_ = reference_->get_resource()->clone_panel();
            owned_panel_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            layout_->addWidget(owned_panel_);

            reference_.reset();

            reference_button_widget_->hide();
            owned_button_widget_    ->show();

            if(dirty_callback_)
                dirty_callback_();
        });

        // delete current panel and select a new one from pool
        signal_to_callback_.connect_callback(owned_select_in_pool, &QPushButton::clicked, [=]
        {
            assert(!reference_ && owned_panel_);

            auto new_ref = pool_.select_resource();
            if(!new_ref)
                return;

            delete owned_panel_;
            owned_panel_ = nullptr;

            reference_ = std::move(new_ref);
            reference_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            reference_->set_removed_callback(reference_removed_callback);

            reference_button_widget_->show();
            owned_button_widget_    ->hide();

            if(dirty_callback_)
                dirty_callback_();
        });

        // add current panel to pool and hold a reference to it
        signal_to_callback_.connect_callback(add_to_pool, &QPushButton::clicked, [=]
        {
            assert(!reference_ && owned_panel_);

            const QString name = QInputDialog::getText(this, "Input Name", "Enter resource name");
            if(!pool_.is_valid_name(name))
            {
                QMessageBox mbox;
                mbox.setWindowTitle("Error");
                mbox.setText("Invalid resource name: " + name);
                mbox.exec();
            }

            auto rsc = pool_.add_resource(name, std::unique_ptr<ResourcePanel<TracerObject>>(owned_panel_));
            owned_panel_ = nullptr;

            reference_ = rsc->create_reference();
            reference_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            reference_->set_removed_callback(reference_removed_callback);

            reference_button_widget_->show();
            owned_button_widget_    ->hide();

            if(dirty_callback_)
                dirty_callback_();
        });

        reference_button_layout->addWidget(create);
        reference_button_layout->addWidget(select_in_pool);
        reference_button_layout->addWidget(edit_rsc);
        reference_button_layout->addWidget(break_link);

        owned_button_layout->addWidget(owned_select_in_pool);
        owned_button_layout->addWidget(add_to_pool);
    }

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(reference_button_widget_);
    layout_->addWidget(owned_button_widget_);
    layout_->addWidget(owned_panel_);

    reference_button_widget_->hide();
}

template<typename TracerObject>
ResourceSlot<TracerObject>::~ResourceSlot()
{
    if(owned_panel_)
        delete owned_panel_;
}

template<typename TracerObject>
void ResourceSlot<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourceSlot<TracerObject>::update_tracer_object()
{
    if(reference_)
        return reference_->update_tracer_object();
    assert(owned_panel_);
    return owned_panel_->update_tracer_object();
}

AGZ_EDITOR_END
