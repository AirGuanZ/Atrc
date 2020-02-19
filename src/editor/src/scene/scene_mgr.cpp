#include <agz/editor/scene/scene_mgr.h>
#include <agz/editor/editor.h>

AGZ_EDITOR_BEGIN

namespace
{
    constexpr int WIDGET_ITEM_HEIGHT = 35;
}

SceneManager::SceneManager(ObjectContext &obj_ctx, Editor *editor, DisplayerGLWidget *displayer_gl)
    : obj_ctx_(obj_ctx), editor_(editor), displayer_gl_(displayer_gl)
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

    connect(displayer_gl_, &DisplayerGLWidget::edit_gizmo, [=](const DirectTransform &new_world)
    {
        edit_entity_gizmo(new_world);
    });

    aggregate_ = tracer::create_native_aggregate();
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
        auto ent = p.second->panel->update_tracer_object();
        entity_arr.push_back(ent);
        entities.push_back(ent);
    }
    aggregate_->build(entity_arr);
    return aggregate_;
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

    auto geometry_data = panel->get_vertices();
    displayer_gl_->add_mesh(mesh_id, geometry_data.data(), static_cast<int>(geometry_data.size()));

    panel->set_geometry_vertices_dirty_callback([=]
    {
        displayer_gl_->remove_mesh(mesh_id);
        auto data = panel->get_vertices();
        displayer_gl_->add_mesh(mesh_id, data.data(), static_cast<int>(data.size()));
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

AGZ_EDITOR_END
