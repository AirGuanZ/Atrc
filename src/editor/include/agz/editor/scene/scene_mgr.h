#pragma once

#include <agz/editor/displayer/preview_window.h>
#include <agz/editor/entity/entity.h>
#include <agz/editor/scene/scene_mgr_ui.h>
#include <agz/utility/mesh.h>

AGZ_EDITOR_BEGIN

class AssetLoader;
class AssetSaver;
class Editor;

class SceneManager : public QObject, public misc::uncopyable_t
{
    Q_OBJECT

public:

    SceneManager(
        ObjectContext &obj_ctx, Editor *editor, PreviewWindow *preview_window);

    ~SceneManager();

    QWidget *get_widget();

    RC<tracer::Aggregate> update_tracer_aggregate(
        std::vector<RC<tracer::Entity>> &entities);

    void add_meshes(const std::vector<mesh::mesh_t> &meshes);

    void save_asset(AssetSaver &saver) const;

    void load_asset(AssetLoader &loader);

    RC<tracer::ConfigArray> to_config(JSONExportContext &ctx) const;

signals:

    void change_scene();

private:

    bool is_valid_name(const QString &name) const;

    void create_entity();

    void remove_selected_entity();

    void rename_selected_entity();

    void selected_entity_changed();

    void edit_entity_gizmo(const DirectTransform &new_world_transform);

    // add single mesh without emit change_scene
    void add_single_mesh(const mesh::mesh_t &mesh);

    void add_record(const QString &name, EntityPanel *entity_panel);

    // convert a (possibly empty) name to a valid name
    QString to_valid_name(const QString &name) const;

    struct Record
    {
        QString                   name;
        EntityPanel              *panel;
        PreviewWindow::MeshID mesh_id;
    };

    ObjectContext &obj_ctx_;
    Editor        *editor_ = nullptr;

    PreviewWindow *preview_window_ = nullptr;

    std::map<QString, Box<Record>> name2record_;

    SceneManagerWidget *ui_ = nullptr;

    RC<tracer::Aggregate> aggregate_;
};

AGZ_EDITOR_END
