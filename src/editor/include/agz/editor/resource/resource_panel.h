#pragma once

#include <QVBoxLayout>

#include <agz/editor/resource/resource_widget.h>
#include <agz/editor/ui/utility/combobox_without_wheel.h>
#include <agz/editor/ui/utility/qsignal_to_callback.h>

AGZ_EDITOR_BEGIN

class ObjectContext;

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

    std::unique_ptr<ResourceThumbnailProvider> get_thumbnail(int width, int height) const;

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

    std::vector<Vertex> get_vertices() const override;

    DirectTransform get_transform() const override;

    void set_transform(const DirectTransform &transform) override;

private:

    void on_change_selected_type();

    ObjectContext &obj_ctx_;

    QSignalToCallback signal_to_callback_;

    QVBoxLayout               *layout_        = nullptr;
    ComboBoxWithoutWheelFocus *type_selector_ = nullptr;

    ResourceWidget<TracerObject> *rsc_widget_ = nullptr;

    std::function<void()> dirty_callback_;
};

AGZ_EDITOR_END
