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

SceneManager::SceneManager(
    ObjectContext &obj_ctx, Editor *editor, PreviewWindow *preview_window)
    : obj_ctx_(obj_ctx), editor_(editor), preview_window_(preview_window)
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

    connect(ui_->name_list, &QListWidget::currentTextChanged,
        [=](const QString&)
    {
        selected_entity_changed();
    });

    connect(preview_window_, &PreviewWindow::left_click_to_emit_ray,
        [=](const Vec3 &o, const Vec3 &d)
    {
        const tracer::Ray ray(o, d);
        real t = (std::numeric_limits<real>::max)();
        Record *selected_record = nullptr;
        for(auto &p : name2record_)
        {
            tracer::EntityIntersection inct;
            if(p.second->panel->get_tracer_object()
                              ->closest_intersection(ray, &inct))
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
                ui_->name_list->findItems(
                    selected_record->name, Qt::MatchExactly).front());
        }
        else
            ui_->name_list->setCurrentItem(nullptr);
    });

    connect(preview_window_, &PreviewWindow::edit_gizmo,
        [=](const DirectTransform &new_world)
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

RC<tracer::Aggregate> SceneManager::update_tracer_aggregate(
    std::vector<RC<tracer::Entity>> &entities)
{
    std::vector<RC<const tracer::Entity>> entity_arr;
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

void SceneManager::save_asset(AssetSaver &saver) const
{
    const uint32_t entity_count = uint32_t(name2record_.size());
    saver.write(entity_count);

    for(uint32_t i = 0; i < entity_count; ++i)
    {
        auto item = ui_->name_list->item(i);
        const QString name = item->text();

        saver.write_string(name);

        auto it = name2record_.find(name);
        assert(it != name2record_.end());
        it->second->panel->save_asset(saver);
    }
}

void SceneManager::load_asset(AssetLoader &loader)
{
    const uint32_t count = loader.read<uint32_t>();
    for(uint32_t i = 0; i < count; ++i)
    {
        // load name & entity panel
        
        const QString name = to_valid_name(loader.read_string());

        EntityPanel *panel = new EntityPanel(obj_ctx_, "Geometric");
        panel->load_asset(loader);

        add_record(name, panel);
    }
}

RC<tracer::ConfigArray> SceneManager::to_config(JSONExportContext &ctx) const
{
    auto arr = newRC<tracer::ConfigArray>();
    for(auto &p : name2record_)
        arr->push_back(p.second->panel->to_config(ctx));
    return arr;
}

bool SceneManager::is_valid_name(const QString &name) const
{
    return name2record_.find(name) == name2record_.end();
}

void SceneManager::create_entity()
{
    // get entity name

    bool ok = false;
    const QString name = to_valid_name(QInputDialog::getText(
        ui_, "Name", "Enter entity name", QLineEdit::Normal, {}, &ok));
    if(!ok)
        return;

    // create entity panel

    EntityPanel *panel = new EntityPanel(obj_ctx_, "Geometric");

    add_record(name, panel);

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

    preview_window_->remove_mesh(it->second->mesh_id);

    name2record_.erase(it);
    delete ui_->name_list->takeItem(ui_->name_list->row(item));

    emit change_scene();
}

void SceneManager::rename_selected_entity()
{
    auto item = ui_->name_list->currentItem();
    if(!item)
        return;

    bool ok = false;
    const QString new_name = to_valid_name(QInputDialog::getText(
        ui_, "Name", "Enter resource name", QLineEdit::Normal, {}, &ok));
    if(!ok)
        return;

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
        preview_window_->set_selected_mesh(-1);
        return;
    }

    auto it = name2record_.find(item->text());
    auto &mesh = *it->second;

    preview_window_->set_selected_mesh(mesh.mesh_id);
    editor_->show_entity_panel(mesh.panel, false);
}

void SceneManager::edit_entity_gizmo(const DirectTransform &transform)
{
    auto item = ui_->name_list->currentItem();
    if(!item)
    {
        preview_window_->set_selected_mesh(-1);
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
    geometry_clone_state.vertices = newRC<std::vector<TriangleBVHWidget::Vertex>>();
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

    GeometricEntityWidget *entity_widget = new GeometricEntityWidget(
        entity_clone_state, obj_ctx_);
    EntityPanel *entity_panel = new EntityPanel(obj_ctx_, entity_widget, "Geometric");

    add_record(QString::fromStdString(mesh.name), entity_panel);
}

void SceneManager::add_record(const ::QString &name, EntityPanel *entity_panel)
{
    // generate mesh id

    auto mesh_id = preview_window_->generate_mesh_id();

    // record name -> entity

    const QString final_name = to_valid_name(name);

    Record record = { final_name, entity_panel, mesh_id };
    name2record_[final_name] = newBox<Record>(record);

    // add entity widget to editor

    editor_->add_to_entity_panel(entity_panel);
    ui_->name_list->addItem(final_name);

    auto new_item = ui_->name_list->findItems(final_name, Qt::MatchExactly).front();
    new_item->setSizeHint(QSize(new_item->sizeHint().width(), WIDGET_ITEM_HEIGHT));

    // set widget callback

    entity_panel->set_dirty_callback([=] { emit change_scene(); });

    // add mesh to gl widget

    const auto geometry_data = entity_panel->get_vertices();
    preview_window_->add_mesh(
        mesh_id, geometry_data.data(), static_cast<int>(geometry_data.size()));

    entity_panel->set_geometry_vertices_dirty_callback([=]
    {
        preview_window_->remove_mesh(mesh_id);
        const auto data = entity_panel->get_vertices();
        preview_window_->add_mesh(
            mesh_id, data.data(), static_cast<int>(data.size()));
        preview_window_->set_mesh_transform(
            mesh_id, entity_panel->get_transform());
    });

    entity_panel->set_entity_transform_dirty_callback([=]
    {
        preview_window_->set_mesh_transform(
            mesh_id, entity_panel->get_transform());
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
