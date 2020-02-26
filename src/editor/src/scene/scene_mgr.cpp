#include <agz/editor/displayer/displayer.h>
#include <agz/editor/editor.h>
#include <agz/editor/entity/geometric_entity.h>
#include <agz/editor/geometry/triangle_bvh.h>
#include <agz/editor/imexport/model_importer.h>
#include <agz/editor/scene/scene_mgr.h>

AGZ_EDITOR_BEGIN

namespace
{
    constexpr int WIDGET_ITEM_HEIGHT = 35;
}

SceneManager::SceneManager(ObjectContext &obj_ctx, Editor *editor, Displayer *displayer)
    : obj_ctx_(obj_ctx), editor_(editor),
      displayer_(displayer), displayer_gl_(displayer_->get_gl_widget())
{
    ui_ = new SceneManagerWidget;

    connect(ui_->create, &QPushButton::clicked, [=]
    {
        create_entity();
    });

    connect(ui_->remove, &QPushButton::clicked, [=]
    {
        remove_selected_entity();
    });

    connect(ui_->rename, &QPushButton::clicked, [=]
    {
        rename_selected_entity();
    });

    connect(ui_->name_list, &QListWidget::currentTextChanged, [=](const QString&)
    {
        selected_entity_changed();
    });

    connect(displayer_, &Displayer::left_button_emit_ray, [=](const Vec3 &o, const Vec3 &d)
    {
        const tracer::Ray ray(o, d);
        real t = (std::numeric_limits<real>::max)();
        Record *selected_record = nullptr;
        for(auto &p : name2record_)
        {
            tracer::EntityIntersection inct;
            if(p.second->panel->get_tracer_object()->closest_intersection(ray, &inct))
            {
                if(inct.t < t)
                {
                    t = inct.t;
                    selected_record = p.second.get();
                }
            }
        }

        if(selected_record)
        {
            ui_->name_list->setCurrentItem(
                ui_->name_list->findItems(selected_record->name, Qt::MatchExactly).front());
        }
        else
            ui_->name_list->setCurrentItem(nullptr);
    });

    connect(displayer_gl_, &DisplayerGLWidget::edit_gizmo, [=](const DirectTransform &new_world)
    {
        edit_entity_gizmo(new_world);
    });

    ModelImporter *model_importer = new ModelImporter(ui_, this);
    connect(ui_->import, &QPushButton::clicked, [=]
    {
        model_importer->exec();
    });

    aggregate_ = tracer::create_entity_bvh(5);
    aggregate_->build({});
}

SceneManager::~SceneManager()
{
    for(auto &p : name2record_)
        delete p.second->panel;
}

QWidget *SceneManager::get_widget()
{
    return ui_;
}

std::shared_ptr<tracer::Aggregate> SceneManager::update_tracer_aggregate(
    std::vector<std::shared_ptr<tracer::Entity>> &entities)
{
    std::vector<std::shared_ptr<const tracer::Entity>> entity_arr;
    for(auto &p : name2record_)
    {
        auto ent = p.second->panel->get_tracer_object();
        entity_arr.push_back(ent);
        entities.push_back(ent);
    }
    aggregate_->build(entity_arr);
    return aggregate_;
}

void SceneManager::add_meshes(const std::vector<mesh::mesh_t> &meshes)
{
    for(auto &m : meshes)
        add_single_mesh(m);
    emit change_scene();
}

bool SceneManager::is_valid_name(const QString &name) const
{
    return name2record_.find(name) == name2record_.end();
}

void SceneManager::create_entity()
{
    // get entity name

    const QString name = QInputDialog::getText(
        ui_, "Name", "Enter entity name");
    if(name.isEmpty())
        return;

    if(!is_valid_name(name))
    {
        QMessageBox::information(ui_, "Error", "Invalid entity name: " + name);
        return;
    }

    // create entity panel

    EntityPanel *panel = new EntityPanel(obj_ctx_, "Geometric");

    // generate mesh id

    auto mesh_id = displayer_gl_->generate_mesh_id();

    // record name -> entity

    Record record = { name, panel, mesh_id };
    name2record_[name] = std::make_unique<Record>(record);

    // add widget to editor

    editor_->add_to_entity_panel(panel);
    ui_->name_list->addItem(name);

    auto new_item = ui_->name_list->findItems(name, Qt::MatchExactly).front();
    new_item->setSizeHint(QSize(new_item->sizeHint().width(), WIDGET_ITEM_HEIGHT));

    // set widget callback

    panel->set_dirty_callback([=] { emit change_scene(); });

    // add mesh to gl widget

    const auto geometry_data = panel->get_vertices();
    displayer_gl_->add_mesh(mesh_id, geometry_data.data(), static_cast<int>(geometry_data.size()));

    panel->set_geometry_vertices_dirty_callback([=]
    {
        displayer_gl_->remove_mesh(mesh_id);
        const auto data = panel->get_vertices();
        displayer_gl_->add_mesh(mesh_id, data.data(), static_cast<int>(data.size()));
        displayer_gl_->set_transform(mesh_id, panel->get_transform());
    });

    panel->set_entity_transform_dirty_callback([=]
    {
        displayer_gl_->set_transform(mesh_id, panel->get_transform());
    });

    emit change_scene();
}

void SceneManager::remove_selected_entity()
{
    auto item = ui_->name_list->currentItem();
    if(!item)
        return;

    auto it = name2record_.find(item->text());
    assert(it != name2record_.end());
    delete it->second->panel;

    displayer_gl_->remove_mesh(it->second->mesh_id);

    name2record_.erase(it);
    delete ui_->name_list->takeItem(ui_->name_list->row(item));

    emit change_scene();
}

void SceneManager::rename_selected_entity()
{
    auto item = ui_->name_list->currentItem();
    if(!item)
        return;

    const QString new_name = QInputDialog::getText(
        ui_, "Name", "Enter resource name");
    if(new_name.isEmpty())
        return;

    if(!is_valid_name(new_name))
    {
        show_invalid_name_mbox(new_name);
        return;
    }

    auto it = name2record_.find(item->text());
    item->setText(new_name);
    
    auto rcd = std::move(it->second);
    rcd->name = new_name;
    name2record_.erase(it);

    name2record_[new_name] = std::move(rcd);
}

void SceneManager::selected_entity_changed()
{
    auto item = ui_->name_list->currentItem();
    if(!item)
    {
        displayer_gl_->set_selected_mesh(-1);
        return;
    }

    auto it = name2record_.find(item->text());
    auto &mesh = *it->second;

    displayer_gl_->set_selected_mesh(mesh.mesh_id);
    editor_->show_entity_panel(mesh.panel, false);
}

void SceneManager::edit_entity_gizmo(const DirectTransform &transform)
{
    auto item = ui_->name_list->currentItem();
    if(!item)
    {
        displayer_gl_->set_selected_mesh(-1);
        return;
    }

    auto it = name2record_.find(item->text());
    auto &mesh = *it->second;

    mesh.panel->set_transform(transform);
}

void SceneManager::add_single_mesh(const mesh::mesh_t &mesh)
{
    TriangleBVHWidget::CloneState geometry_clone_state;
    geometry_clone_state.filename = "";
    geometry_clone_state.vertices = std::make_shared<std::vector<TriangleBVHWidget::Vertex>>();
    geometry_clone_state.vertices->reserve(3 * mesh.triangles.size());
    for(auto &tri : mesh.triangles)
    {
        for(int i = 0; i < 3; ++i)
        {
            geometry_clone_state.vertices->push_back({
                tri.vertices[i].position,
                tri.vertices[i].normal,
                tri.vertices[i].tex_coord
            });
        }
    }
    auto geometry_widget = new TriangleBVHWidget(geometry_clone_state);
    GeometrySlot *geometry_slot = new GeometrySlot(
        obj_ctx_, "Triangle Mesh", geometry_widget, "Triangle Mesh");

    GeometricEntityWidget::CloneState entity_clone_state;
    entity_clone_state.geometry = geometry_slot;

    GeometricEntityWidget *entity_widget = new GeometricEntityWidget(entity_clone_state, obj_ctx_);
    EntityPanel *entity_panel = new EntityPanel(obj_ctx_, entity_widget, "Geometric");

    // generate mesh id

    auto mesh_id = displayer_gl_->generate_mesh_id();

    // record name -> entity

    const QString final_name = to_valid_name(QString::fromStdString(mesh.name));

    Record record = { final_name, entity_panel, mesh_id };
    name2record_[final_name] = std::make_unique<Record>(record);

    // add entity widget to editor

    editor_->add_to_entity_panel(entity_panel);
    ui_->name_list->addItem(final_name);

    auto new_item = ui_->name_list->findItems(final_name, Qt::MatchExactly).front();
    new_item->setSizeHint(QSize(new_item->sizeHint().width(), WIDGET_ITEM_HEIGHT));

    // set widget callback

    entity_panel->set_dirty_callback([=] { emit change_scene(); });

    // add mesh to gl widget

    const auto geometry_data = entity_panel->get_vertices();
    displayer_gl_->add_mesh(mesh_id, geometry_data.data(), static_cast<int>(geometry_data.size()));

    entity_panel->set_geometry_vertices_dirty_callback([=]
    {
        displayer_gl_->remove_mesh(mesh_id);
        const auto data = entity_panel->get_vertices();
        displayer_gl_->add_mesh(mesh_id, data.data(), static_cast<int>(data.size()));
        displayer_gl_->set_transform(mesh_id, entity_panel->get_transform());
    });

    entity_panel->set_entity_transform_dirty_callback([=]
    {
        displayer_gl_->set_transform(mesh_id, entity_panel->get_transform());
    });
}

QString SceneManager::to_valid_name(const QString &name) const
{
    if(name.isEmpty())
        return to_valid_name("auto");

    if(is_valid_name(name))
        return name;

    for(int index = 0;; ++index)
    {
        QString ret = QString("%1 (%2)").arg(name).arg(index);
        if(is_valid_name(ret))
            return ret;
    }
}

AGZ_EDITOR_END
