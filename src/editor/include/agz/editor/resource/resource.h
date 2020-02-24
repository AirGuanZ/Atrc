#pragma once

#include <map>
#include <set>

#include <QInputDialog>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

#include <agz/editor/resource/entity_interface.h>
#include <agz/editor/ui/utility/combobox_without_wheel.h>
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

class Editor;
class ObjectContext;

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
class ResourceWidget : public QWidget, public EntityInterface
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
    std::shared_ptr<TracerObject> get_tracer_object();

    /**
     * @brief clone the resource widget and tracer object
     */
    virtual ResourceWidget<TracerObject> *clone() = 0;

    /**
     * @brief get the thumbnail image
     */
    virtual QPixmap get_thumbnail(int width, int height) const = 0;

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

    virtual ResourceWidget<TracerObject> *create_widget(ObjectContext &obj_ctx) const = 0;
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

    Widget *create_widget(const QString &name, ObjectContext &obj_ctx) const;

private:

    QStringList type_names_;

    std::map<QString, std::unique_ptr<Creator>, std::less<>> name2creator_;
};

/**
 * @brief type selector + resource widget
 */
template<typename TracerObject>
class ResourcePanel : public QWidget, public EntityInterface
{
public:

    explicit ResourcePanel(ObjectContext &obj_ctx, const QString &default_type);

    ResourcePanel(
        ObjectContext &obj_ctx,
        ResourceWidget<TracerObject> *rsc_widget, const QString &current_type_name);

    void set_dirty_callback(std::function<void()> callback);

    std::shared_ptr<TracerObject> get_tracer_object();

    ResourcePanel<TracerObject> *clone() const;

    QPixmap get_thumbnail(int width, int height) const;

    std::vector<Vertex> get_vertices() const override;

    DirectTransform get_transform() const override;

    void set_transform(const DirectTransform &transform) override;

private:

    ObjectContext &obj_ctx_;

    QSignalToCallback signal_to_callback_;

    QVBoxLayout               *layout_        = nullptr;
    ComboBoxWithoutWheelFocus *type_selector_ = nullptr;

    ResourceWidget<TracerObject> *rsc_widget_ = nullptr;

    std::function<void()> dirty_callback_;
};

/**
 * @brief reference to a resource instance in pool
 *
 * when the resource is destroyed, all its references's `removed_callback` is automatically called
 */
template<typename TracerObject>
class ResourceReference : public misc::uncopyable_t, public EntityInterface
{
public:

    explicit ResourceReference(ResourceInPool<TracerObject> *rsc_in_pool);

    ~ResourceReference();

    void set_dirty_callback(std::function<void()> callback);

    void set_removed_callback(std::function<void()> callback);

    std::shared_ptr<TracerObject> get_tracer_object();

    ResourceInPool<TracerObject> *get_resource();

    QWidget *get_name_widget();

    std::vector<Vertex> get_vertices() const override;

    DirectTransform get_transform() const override;

    void set_transform(const DirectTransform &transform) override;

private:

    friend class ResourceInPool<TracerObject>;

    void set_name(const QString &name);

    void call_dirty_callback();

    void call_remove_callback();

    std::function<void()> dirty_callback_;
    std::function<void()> removed_callback_;

    QString name_;
    QPointer<QLabel> name_label_;

    ResourceInPool<TracerObject> *rsc_in_pool_;
};

/**
 * @brief resource instance in pool
 *
 * when an instance is destroyed, all its references's 'removed_callback' is automatically called.
 * After that, those references become invalid.
 */
template<typename TracerObject>
class ResourceInPool : public misc::uncopyable_t, public EntityInterface
{
public:

    explicit ResourceInPool(const QString &name, std::unique_ptr<ResourcePanel<TracerObject>> panel);

    ResourceInPool(const QString &name, const ResourceWidgetFactory<TracerObject> &factory, const QString &default_type);

    ~ResourceInPool();

    void set_dirty_callback(std::function<void()> callback);

    std::unique_ptr<ResourceReference<TracerObject>> create_reference();

    std::shared_ptr<TracerObject> get_tracer_object();

    ResourcePanel<TracerObject> *get_panel();

    ResourcePanel<TracerObject> *clone_panel();

    QPixmap get_thumbnail(int width, int height) const;

    QString get_name() const;

    void set_name(const QString &name);

    std::vector<Vertex> get_vertices() const override;

    DirectTransform get_transform() const override;

    void set_transform(const DirectTransform &transform) override;

private:

    friend class ResourceReference<TracerObject>;

    void remove_reference(ResourceReference<TracerObject> *ref);

    std::function<void()> dirty_callback_;

    QString name_;

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
        const QString &name, std::unique_ptr<ResourcePanel<TracerObject>> panel) = 0;

    virtual bool is_valid_name(const QString &name) const = 0;

    virtual void show_edit_panel(ResourcePanel<TracerObject> *rsc, bool display_rsc_panel) = 0;

    virtual QWidget *get_widget() = 0;
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
class ResourceSlot : public QWidget, public misc::uncopyable_t, public EntityInterface
{
public:

    ResourceSlot(ObjectContext &obj_ctx, const QString &default_type);

    ResourceSlot(
        ObjectContext &obj_ctx, const QString &default_type,
        ResourceWidget<TracerObject> *widget, const QString &widget_type);

    ~ResourceSlot();

    void set_dirty_callback(std::function<void()> callback);

    std::shared_ptr<TracerObject> get_tracer_object();

    ResourceSlot<TracerObject> *clone() const;

    std::vector<Vertex> get_vertices() const override;

    DirectTransform get_transform() const override;

    void set_transform(const DirectTransform &transform) override;

private:

    ResourceSlot(
        ObjectContext &obj_ctx, const QString &default_type,
        std::unique_ptr<ResourceReference<TracerObject>> reference,
        ResourcePanel<TracerObject> *owned_panel);

    QSignalToCallback signal_to_callback_;

    ObjectContext &obj_ctx_;

    QString default_panel_type_;

    std::function<void()> dirty_callback_;

    QVBoxLayout *layout_ = nullptr;

    // exactly one of `reference_` and `owned_panel_` is non-null

    std::unique_ptr<ResourceReference<TracerObject>> reference_;
    ResourcePanel<TracerObject> *owned_panel_ = nullptr;

    QPointer<QWidget> reference_widget_;
    QPointer<QWidget> owned_button_widget_;
};

class ObjectContext
{
    template<typename...TracerObjects>
    using FactoryTuple = std::tuple<ResourceWidgetFactory<TracerObjects>...>;

    template<typename...TracerObjects>
    using PoolTuple = std::tuple<std::unique_ptr<ResourcePool<TracerObjects>>...>;

    template <typename T, typename Tuple>
    struct has_type;

    template <typename T, typename... Us>
    struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

    FactoryTuple<
        tracer::Entity,
        tracer::EnvirLight,
        tracer::Geometry,
        tracer::Material,
        tracer::Texture2D> factorys_;

    PoolTuple<
        tracer::Geometry,
        tracer::Material,
        tracer::Texture2D> pools_;

public:

    explicit ObjectContext(Editor *editor);

    template<typename TracerObject>
    const ResourceWidgetFactory<TracerObject> &factory() const noexcept
    {
        return std::get<ResourceWidgetFactory<TracerObject>>(factorys_);
    }

    template<typename TracerObject>
    const ResourcePool<TracerObject> *pool() const noexcept
    {
        if constexpr(!has_type<std::unique_ptr<ResourcePool<TracerObject>>, decltype(pools_)>::value)
            return nullptr;
        else
            return std::get<std::unique_ptr<ResourcePool<TracerObject>>>(pools_).get();
    }

    template<typename TracerObject>
    ResourceWidgetFactory<TracerObject> &factory() noexcept
    {
        return std::get<ResourceWidgetFactory<TracerObject>>(factorys_);
    }

    template<typename TracerObject>
    ResourcePool<TracerObject> *pool() noexcept
    {
        if constexpr(!has_type<std::unique_ptr<ResourcePool<TracerObject>>, decltype(pools_)>::value)
            return nullptr;
        else
            return std::get<std::unique_ptr<ResourcePool<TracerObject>>>(pools_).get();
    }
};

inline void show_invalid_name_mbox(const QString &name)
{
    QMessageBox mbox;
    mbox.setWindowTitle("Error");
    mbox.setText("Invalid resource name: " + name);
    mbox.exec();
}

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
std::shared_ptr<TracerObject> ResourceWidget<TracerObject>::get_tracer_object()
{
    return tracer_object_;
}

template<typename TracerObject>
void ResourceWidget<TracerObject>::set_dirty_flag()
{
    update_tracer_object_impl();
    if(dirty_callback_)
        dirty_callback_();
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
    ResourceWidgetFactory<TracerObject>::create_widget(const QString &name, ObjectContext &obj_ctx) const
{
    const auto it = name2creator_.find(name);
    if(it == name2creator_.end())
        throw ResourceObjectException("unknown resource widget creator name: " + name);
    return it->second->create_widget(obj_ctx);
}

template<typename TracerObject>
ResourcePanel<TracerObject>::ResourcePanel(ObjectContext &obj_ctx, const QString &default_type)
    : obj_ctx_(obj_ctx)
{
    type_selector_ = new ComboBoxWithoutWheelFocus(this);
    type_selector_->addItems(obj_ctx.factory<TracerObject>().get_type_names());
    type_selector_->setCurrentText(default_type);

    auto change_type = [=](const QString &new_type)
    {
        if(rsc_widget_)
            delete rsc_widget_;

        rsc_widget_ = obj_ctx_.factory<TracerObject>().create_widget(new_type, obj_ctx_);
        rsc_widget_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });
        rsc_widget_->set_geometry_vertices_dirty_callback([=]
        {
            set_geometry_vertices_dirty();
        });
        rsc_widget_->set_entity_transform_dirty_callback([=]
        {
            set_entity_transform_dirty();
        });

        layout_->addWidget(rsc_widget_);

        if(dirty_callback_)
            dirty_callback_();

        set_geometry_vertices_dirty();
        set_entity_transform_dirty();
    };

    signal_to_callback_.connect_callback(
        type_selector_, &ComboBoxWithoutWheelFocus::currentTextChanged, change_type);

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(type_selector_);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);

    change_type(default_type);
}

template<typename TracerObject>
ResourcePanel<TracerObject>::ResourcePanel(
    ObjectContext &obj_ctx,
    ResourceWidget<TracerObject> *rsc_widget, const QString &current_type_name)
    : obj_ctx_(obj_ctx)
{
    type_selector_ = new ComboBoxWithoutWheelFocus(this);
    type_selector_->addItems(obj_ctx.factory<TracerObject>().get_type_names());
    type_selector_->setCurrentText(current_type_name);

    signal_to_callback_.connect_callback(
        type_selector_, &ComboBoxWithoutWheelFocus::currentTextChanged, [=](const QString &new_type)
    {
        if(rsc_widget_)
            delete rsc_widget_;

        rsc_widget_ = obj_ctx_.factory<TracerObject>().create_widget(new_type, obj_ctx_);
        rsc_widget_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });
        rsc_widget_->set_geometry_vertices_dirty_callback([=]
        {
            set_geometry_vertices_dirty();
        });
        rsc_widget_->set_entity_transform_dirty_callback([=]
        {
            set_entity_transform_dirty();
        });

        layout_->addWidget(rsc_widget_);

        if(dirty_callback_)
            dirty_callback_();

        set_geometry_vertices_dirty();
        set_entity_transform_dirty();
    });

    rsc_widget_ = rsc_widget;
    rsc_widget_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
    });
    rsc_widget_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
    });
    rsc_widget_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
    });

    layout_ = new QVBoxLayout(this);
    layout_->addWidget(type_selector_);
    layout_->addWidget(rsc_widget_);

    setContentsMargins(0, 0, 0, 0);
    layout_->setContentsMargins(0, 0, 0, 0);
}

template<typename TracerObject>
void ResourcePanel<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourcePanel<TracerObject>::get_tracer_object()
{
    return rsc_widget_->get_tracer_object();
}

template<typename TracerObject>
ResourcePanel<TracerObject> *ResourcePanel<TracerObject>::clone() const
{
    return new ResourcePanel(obj_ctx_, rsc_widget_->clone(), type_selector_->currentText());
}

template<typename TracerObject>
QPixmap ResourcePanel<TracerObject>::get_thumbnail(int width, int height) const
{
    return rsc_widget_->get_thumbnail(width, height);
}

template<typename TracerObject>
std::vector<EntityInterface::Vertex> ResourcePanel<TracerObject>::get_vertices() const
{
    return rsc_widget_->get_vertices();
}

template<typename TracerObject>
DirectTransform ResourcePanel<TracerObject>::get_transform() const
{
    return rsc_widget_->get_transform();
}

template<typename TracerObject>
void ResourcePanel<TracerObject>::set_transform(const DirectTransform &transform)
{
    rsc_widget_->set_transform(transform);
}

template<typename TracerObject>
ResourceReference<TracerObject>::ResourceReference(ResourceInPool<TracerObject> *rsc_in_pool)
    : rsc_in_pool_(rsc_in_pool)
{
    assert(rsc_in_pool);
    name_ = rsc_in_pool_->get_name();
}

template<typename TracerObject>
ResourceReference<TracerObject>::~ResourceReference()
{
    if(rsc_in_pool_)
        rsc_in_pool_->remove_reference(this);
    if(name_label_)
        delete name_label_.data();
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
std::shared_ptr<TracerObject> ResourceReference<TracerObject>::get_tracer_object()
{
    assert(rsc_in_pool_);
    return rsc_in_pool_->get_tracer_object();
}

template<typename TracerObject>
ResourceInPool<TracerObject> *ResourceReference<TracerObject>::get_resource()
{
    assert(rsc_in_pool_);
    return rsc_in_pool_;
}

template<typename TracerObject>
QWidget *ResourceReference<TracerObject>::get_name_widget()
{
    if(name_label_)
        return name_label_;
    name_label_ = new QLabel(name_);
    name_label_->setAlignment(Qt::AlignCenter);
    return name_label_;
}

template<typename TracerObject>
std::vector<EntityInterface::Vertex> ResourceReference<TracerObject>::get_vertices() const
{
    return rsc_in_pool_->get_vertices();
}

template<typename TracerObject>
DirectTransform ResourceReference<TracerObject>::get_transform() const
{
    return rsc_in_pool_->get_transform();
}

template<typename TracerObject>
void ResourceReference<TracerObject>::set_transform(const DirectTransform &transform)
{
    rsc_in_pool_->set_transform(transform);
}

template<typename TracerObject>
void ResourceReference<TracerObject>::set_name(const QString &name)
{
    name_ = name;
    if(name_label_)
        name_label_->setText(name);
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
ResourceInPool<TracerObject>::ResourceInPool(const QString &name, std::unique_ptr<ResourcePanel<TracerObject>> panel)
    : name_(name)
{
    panel_ = panel.release();
    panel_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
        for(auto ref : references_)
            ref->call_dirty_callback();
    });
    panel_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
        for(auto ref : references_)
            ref->set_geometry_vertices_dirty();
    });
    panel_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
        for(auto ref : references_)
            ref->set_entity_transform_dirty();
    });
}

template<typename TracerObject>
ResourceInPool<TracerObject>::ResourceInPool(
    const QString &name,
    const ResourceWidgetFactory<TracerObject> &factory, const QString &default_type)
    : name_(name)
{
    panel_ = new ResourcePanel<TracerObject>(factory, default_type);
    panel_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
        for(auto ref : references_)
            ref->call_dirty_callback();
    });
    panel_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
        for(auto ref : references_)
            ref->set_geometry_vertices_dirty();
    });
    panel_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
        for(auto ref : references_)
            ref->set_entity_transform_dirty();
    });
}

template<typename TracerObject>
ResourceInPool<TracerObject>::~ResourceInPool()
{
    while(!references_.empty())
    {
        auto ref = *references_.begin();
        ref->call_remove_callback();
        delete ref;
    }

    ResourcePanel<TracerObject> *panel = panel_;
    delete panel;
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
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
std::shared_ptr<TracerObject> ResourceInPool<TracerObject>::get_tracer_object()
{
    return panel_->get_tracer_object();
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
QPixmap ResourceInPool<TracerObject>::get_thumbnail(int width, int height) const
{
    assert(panel_);
    return panel_->get_thumbnail(width, height);
}

template<typename TracerObject>
QString ResourceInPool<TracerObject>::get_name() const
{
    return name_;
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::set_name(const QString &name)
{
    name_ = name;
    for(auto ref : references_)
        ref->set_name(name);
}

template<typename TracerObject>
std::vector<EntityInterface::Vertex> ResourceInPool<TracerObject>::get_vertices() const
{
    return panel_->get_vertices();
}

template<typename TracerObject>
DirectTransform ResourceInPool<TracerObject>::get_transform() const
{
    return panel_->get_transform();
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::set_transform(const DirectTransform &transform)
{
    panel_->set_transform(transform);
}

template<typename TracerObject>
ResourceSlot<TracerObject>::ResourceSlot(
    ObjectContext &obj_ctx, const QString &default_type)
    : ResourceSlot(obj_ctx, default_type, nullptr, new ResourcePanel<TracerObject>(obj_ctx, default_type))
{

}

template<typename TracerObject>
ResourceSlot<TracerObject>::ResourceSlot(
    ObjectContext &obj_ctx, const QString &default_type,
    ResourceWidget<TracerObject> *widget, const QString &widget_type)
    : ResourceSlot(
        obj_ctx, default_type, nullptr,
        new ResourcePanel<TracerObject>(obj_ctx, widget, widget_type))
{

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
std::shared_ptr<TracerObject> ResourceSlot<TracerObject>::get_tracer_object()
{
    if(reference_)
        return reference_->get_tracer_object();
    assert(owned_panel_);
    return owned_panel_->get_tracer_object();
}

template<typename TracerObject>
ResourceSlot<TracerObject> *ResourceSlot<TracerObject>::clone() const
{
    std::unique_ptr<ResourceReference<TracerObject>> reference;
    if(reference_)
        reference = reference_->get_resource()->create_reference();

    ResourcePanel<TracerObject> *owned_panel = nullptr;
    if(owned_panel_)
        owned_panel = owned_panel_->clone();

    return new ResourceSlot<TracerObject>(
        obj_ctx_, default_panel_type_, std::move(reference), owned_panel);
}

template<typename TracerObject>
std::vector<EntityInterface::Vertex> ResourceSlot<TracerObject>::get_vertices() const
{
    if(reference_)
        return reference_->get_vertices();
    return owned_panel_->get_vertices();
}

template<typename TracerObject>
DirectTransform ResourceSlot<TracerObject>::get_transform() const
{
    if(reference_)
        return reference_->get_transform();
    return owned_panel_->get_transform();
}

template<typename TracerObject>
void ResourceSlot<TracerObject>::set_transform(const DirectTransform &transform)
{
    if(reference_)
        reference_->set_transform(transform);
    else
        owned_panel_->set_transform(transform);
}

template<typename TracerObject>
ResourceSlot<TracerObject>::ResourceSlot(
    ObjectContext &obj_ctx, const QString &default_type,
    std::unique_ptr<ResourceReference<TracerObject>> reference, ResourcePanel<TracerObject> *owned_panel)
    : obj_ctx_(obj_ctx), default_panel_type_(default_type)
{
    auto pool = obj_ctx_.pool<TracerObject>();
    const bool has_pool = pool != nullptr;

    owned_panel_ = owned_panel;
    reference_ = std::move(reference);

    if(owned_panel_)
    {
        owned_panel_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });
        owned_panel_->set_geometry_vertices_dirty_callback([=]
        {
            set_geometry_vertices_dirty();
        });
        owned_panel_->set_entity_transform_dirty_callback([=]
        {
            set_entity_transform_dirty();
        });
    }
    else
    {
        assert(reference_);
        reference_->set_dirty_callback([=]
        {
            if(dirty_callback_)
                dirty_callback_();
        });
        reference_->set_geometry_vertices_dirty_callback([=]
        {
            set_geometry_vertices_dirty();
        });
        reference_->set_entity_transform_dirty_callback([=]
        {
            set_entity_transform_dirty();
        });
    }

    if(has_pool)
    {
        reference_widget_ = new QWidget(this);
        owned_button_widget_ = new QWidget(this);

        QVBoxLayout *reference_layout        = new QVBoxLayout(reference_widget_);
        QWidget     *reference_button_widget = new QWidget(reference_widget_);
        QHBoxLayout *reference_button_layout = new QHBoxLayout(reference_button_widget);
        QHBoxLayout *owned_button_layout     = new QHBoxLayout(owned_button_widget_);

        reference_widget_->setContentsMargins(0, 0, 0, 0);
        reference_button_widget->setContentsMargins(0, 0, 0, 0);
        owned_button_widget_->setContentsMargins(0, 0, 0, 0);

        if(reference_)
            reference_layout->addWidget(reference_->get_name_widget());
        reference_layout->addWidget(reference_button_widget);

        reference_layout->setContentsMargins(0, 0, 0, 0);
        reference_button_layout->setContentsMargins(0, 0, 0, 0);
        owned_button_layout->setContentsMargins(0, 0, 0, 0);
        
        QPushButton *create         = new QPushButton("Create", reference_button_widget);
        QPushButton *select_in_pool = new QPushButton("Select", reference_button_widget);
        QPushButton *edit_rsc       = new QPushButton("Edit",   reference_button_widget);
        QPushButton *break_link     = new QPushButton("Unlink", reference_button_widget);

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
            owned_panel_->set_geometry_vertices_dirty_callback([=]
            {
                set_geometry_vertices_dirty();
            });
            owned_panel_->set_entity_transform_dirty_callback([=]
            {
                set_entity_transform_dirty();
            });
            layout_->addWidget(owned_panel_);

            (void)reference_.release();
            
            reference_widget_->hide();
            owned_button_widget_->show();

            if(dirty_callback_)
                dirty_callback_();
            set_geometry_vertices_dirty();
            set_entity_transform_dirty();
        };

        // delete the reference and create a new panel
        signal_to_callback_.connect_callback(create, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);

            reference_.reset();

            owned_panel_ = new ResourcePanel<TracerObject>(obj_ctx_, default_panel_type_);
            owned_panel_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            owned_panel_->set_geometry_vertices_dirty_callback([=]
            {
                set_geometry_vertices_dirty();
            });
            owned_panel_->set_entity_transform_dirty_callback([=]
            {
                set_entity_transform_dirty();
            });
            layout_->addWidget(owned_panel_);

            reference_widget_   ->hide();
            owned_button_widget_->show();

            if(dirty_callback_)
                dirty_callback_();

            set_geometry_vertices_dirty();
            set_entity_transform_dirty();
        });

        // delete current reference and select a new one from pool
        signal_to_callback_.connect_callback(select_in_pool, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);

            auto new_ref = obj_ctx_.pool<TracerObject>()->select_resource();
            if(!new_ref)
                return;

            reference_ = std::move(new_ref);
            reference_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            reference_->set_geometry_vertices_dirty_callback([=]
            {
                set_geometry_vertices_dirty();
            });
            reference_->set_entity_transform_dirty_callback([=]
            {
                set_entity_transform_dirty();
            });
            reference_->set_removed_callback(reference_removed_callback);

            reference_layout->insertWidget(0, reference_->get_name_widget());

            if(dirty_callback_)
                dirty_callback_();
            set_geometry_vertices_dirty();
            set_entity_transform_dirty();
        });

        // edit the referenced resource
        signal_to_callback_.connect_callback(edit_rsc, &QPushButton::clicked, [=]
        {
            assert(reference_ && !owned_panel_);
            obj_ctx_.pool<TracerObject>()->show_edit_panel(reference_->get_resource()->get_panel(), true);
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
            owned_panel_->set_geometry_vertices_dirty_callback([=]
            {
                set_geometry_vertices_dirty();
            });
            owned_panel_->set_entity_transform_dirty_callback([=]
            {
                set_entity_transform_dirty();
            });
            layout_->addWidget(owned_panel_);

            reference_.reset();

            reference_widget_   ->hide();
            owned_button_widget_->show();

            if(dirty_callback_)
                dirty_callback_();

            set_geometry_vertices_dirty();
            set_entity_transform_dirty();
        });

        // delete current panel and select a new one from pool
        signal_to_callback_.connect_callback(owned_select_in_pool, &QPushButton::clicked, [=]
        {
            assert(!reference_ && owned_panel_);

            auto new_ref = obj_ctx_.pool<TracerObject>()->select_resource();
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
            reference_->set_geometry_vertices_dirty_callback([=]
            {
                set_geometry_vertices_dirty();
            });
            reference_->set_entity_transform_dirty_callback([=]
            {
                set_entity_transform_dirty();
            });
            reference_->set_removed_callback(reference_removed_callback);

            reference_widget_   ->show();
            owned_button_widget_->hide();

            reference_layout->insertWidget(0, reference_->get_name_widget());

            if(dirty_callback_)
                dirty_callback_();

            set_geometry_vertices_dirty();
            set_entity_transform_dirty();
        });

        // add current panel to pool and hold a reference to it
        signal_to_callback_.connect_callback(add_to_pool, &QPushButton::clicked, [=]
        {
            assert(!reference_ && owned_panel_);

            const QString name = QInputDialog::getText(this, "Name", "Enter resource name");
            if(name.isEmpty())
                return;

            if(!obj_ctx_.pool<TracerObject>()->is_valid_name(name))
            {
                show_invalid_name_mbox(name);
                return;
            }

            auto rsc = obj_ctx_.pool<TracerObject>()->add_resource(name, std::unique_ptr<ResourcePanel<TracerObject>>(owned_panel_));
            owned_panel_ = nullptr;

            reference_ = rsc->create_reference();
            reference_->set_dirty_callback([=]
            {
                if(dirty_callback_)
                    dirty_callback_();
            });
            reference_->set_geometry_vertices_dirty_callback([=]
            {
                set_geometry_vertices_dirty();
            });
            reference_->set_entity_transform_dirty_callback([=]
            {
                set_entity_transform_dirty();
            });
            reference_->set_removed_callback(reference_removed_callback);

            reference_widget_   ->show();
            owned_button_widget_->hide();

            reference_layout->insertWidget(0, reference_->get_name_widget());

            if(dirty_callback_)
                dirty_callback_();

            set_geometry_vertices_dirty();
            set_entity_transform_dirty();
        });

        reference_button_layout->addWidget(create);
        reference_button_layout->addWidget(select_in_pool);
        reference_button_layout->addWidget(edit_rsc);
        reference_button_layout->addWidget(break_link);

        owned_button_layout->addWidget(owned_select_in_pool);
        owned_button_layout->addWidget(add_to_pool);
    }

    layout_ = new QVBoxLayout(this);
    if(has_pool)
    {
        layout_->addWidget(reference_widget_);
        layout_->addWidget(owned_button_widget_);
        if(owned_panel_)
            reference_widget_->hide();
        else
            owned_button_widget_->hide();
    }
    layout_->addWidget(owned_panel_);
}

AGZ_EDITOR_END
