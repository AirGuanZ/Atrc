#pragma once

#include <agz/editor/displayer/gl_widget.h>
#include <agz/editor/entity/entity.h>
#include <agz/editor/scene/scene_mgr_ui.h>

AGZ_EDITOR_BEGIN

class DisplayerGLWidget;
class Editor;

class SceneManager : public QObject, public misc::uncopyable_t
{
    Q_OBJECT

public:

    SceneManager(ObjectContext &obj_ctx, Editor *editor, DisplayerGLWidget *displayer_gl);

    ~SceneManager();

    QWidget *get_widget();

    std::shared_ptr<tracer::Aggregate> update_tracer_aggregate(std::vector<std::shared_ptr<tracer::Entity>> &entities);

    void add_meshes(const std::vector<mesh::mesh_t> &meshes);

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

    // convert a (possibly empty) name to a valid name
    QString to_valid_name(const QString &name) const;

    struct Record
    {
        QString                   name;
        EntityPanel              *panel;
        DisplayerGLWidget::MeshID mesh_id;
    };

    ObjectContext     &obj_ctx_;
    Editor            *editor_ = nullptr;
    DisplayerGLWidget *displayer_gl_ = nullptr;

    std::map<QString, std::unique_ptr<Record>> name2record_;

    SceneManagerWidget *ui_ = nullptr;

    std::shared_ptr<tracer::Aggregate> aggregate_;
};

AGZ_EDITOR_END
