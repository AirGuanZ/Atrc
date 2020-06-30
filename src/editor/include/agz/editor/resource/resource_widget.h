#pragma once

#include <functional>
#include <map>
#include <memory>

#include <QWidget>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/resource/entity_interface.h>
#include <agz/editor/resource/thumbnail_provider.h>
#include <agz/tracer/utility/config.h>

AGZ_EDITOR_BEGIN

class ObjectContext;
class JSONExportContext;

class ResourceObjectException : public std::exception
{
public:

    explicit ResourceObjectException(const QString &msg);

    char const *what() const noexcept override;

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
    RC<TracerObject> get_tracer_object();

    /**
     * @brief clone the resource widget and tracer object
     */
    virtual ResourceWidget<TracerObject> *clone() = 0;

    /**
     * @brief get the thumbnail image
     */
    virtual Box<ResourceThumbnailProvider> get_thumbnail(
        int width, int height) const;

    /**
     * @brief serialize
     */
    virtual void save_asset(AssetSaver &saver) = 0;

    /**
     * @brief deserialize
     */
    virtual void load_asset(AssetLoader &loader) = 0;

    /**
     * @brief convert to tracer config node
     */
    virtual RC<tracer::ConfigNode> to_config(JSONExportContext &ctx) const = 0;

protected:

    /**
     * @brief set the dirty flag and call the dirty_callback function (if presents)
     */
    void set_dirty_flag();

    /**
     * @brief update the corresponding tracer object
     */
    virtual void update_tracer_object_impl() = 0;

    RC<TracerObject> tracer_object_;
    
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

    virtual ResourceWidget<TracerObject> *create_widget(
        ObjectContext &obj_ctx) const = 0;
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

    void add_creator(Box<Creator> creator);

    const QStringList &get_type_names() const noexcept;

    Widget *create_widget(const QString &name, ObjectContext &obj_ctx) const;

private:

    QStringList type_names_;

    std::map<QString, Box<Creator>, std::less<>> name2creator_;
};


AGZ_EDITOR_END
