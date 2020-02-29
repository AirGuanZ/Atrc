#pragma once

#include <QPointer>

#include <agz/editor/resource/resource_pool.h>

AGZ_EDITOR_BEGIN

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

    void set_reference(std::unique_ptr<ResourceReference<TracerObject>> reference);

    void set_dirty_callback(std::function<void()> callback);

    std::shared_ptr<TracerObject> get_tracer_object();

    ResourceSlot<TracerObject> *clone() const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

    std::vector<Vertex> get_vertices() const override;

    DirectTransform get_transform() const override;

    void set_transform(const DirectTransform &transform) override;

private:

    ResourceSlot(
        ObjectContext &obj_ctx, const QString &default_type,
        std::unique_ptr<ResourceReference<TracerObject>> reference,
        ResourcePanel<TracerObject> *owned_panel);

    // callback when the referenced resource is removed from pool
    // clone the referenced resource as my own panel and destroy the reference
    void reference_removed_callback_impl();

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

    QVBoxLayout *reference_layout_ = nullptr;
};

AGZ_EDITOR_END
