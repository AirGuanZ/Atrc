#pragma once

#include <set>

#include <QLabel>

#include <agz/editor/resource/resource_panel.h>

AGZ_EDITOR_BEGIN

template<typename TracerObject>
class ResourceInPool;

template<typename TracerObject>
class ResourcePool;

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

    std::unique_ptr<ResourceThumbnailProvider> get_thumbnail(int width, int height) const;

    const QString &get_name() const;

    void set_name(const QString &name);

    void save_asset(AssetSaver &saver) const;

    void load_asset(ResourcePool<TracerObject> &pool, AssetLoader &loader);

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

    virtual void save_asset(AssetSaver &saver) const = 0;

    virtual void load_asset(AssetLoader &loader) = 0;

    virtual ResourceInPool<TracerObject> *name_to_rsc(const QString &name) = 0;

    virtual bool is_valid_name(const QString &name) const = 0;

    virtual QString to_valid_name(const QString &name) const = 0;

    virtual void show_edit_panel(ResourcePanel<TracerObject> *rsc, bool display_rsc_panel) = 0;

    virtual QWidget *get_widget() = 0;
};

AGZ_EDITOR_END
