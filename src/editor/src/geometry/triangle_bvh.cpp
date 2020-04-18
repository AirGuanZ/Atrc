#include <QFileDialog>
#include <QMessageBox>

#include <agz/editor/geometry/triangle_bvh.h>
#include <agz/editor/imexport/json_export_context.h>
#include <agz/editor/ui/utility/elided_label.h>
#include <agz/factory/factory.h>
#include <agz/tracer/create/geometry.h>

AGZ_EDITOR_BEGIN

TriangleBVHWidget::TriangleBVHWidget(const CloneState &clone_state)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);

    QWidget     *filename_widget = new QWidget(this);
    QHBoxLayout *filename_layout = new QHBoxLayout(filename_widget);
    QPushButton *filename_browse = new QPushButton("Browse", filename_widget);
    filename_label_ = new ElidedLabel(clone_state.filename, filename_widget);

    filename_layout->addWidget(filename_browse);
    filename_layout->addWidget(filename_label_);
    filename_widget->setContentsMargins(0, 0, 0, 0);
    filename_layout->setContentsMargins(0, 0, 0, 0);

    filename_browse->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(filename_widget);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    filename_      = clone_state.filename;
    tracer_object_ = clone_state.tracer_object;
    vertices_      = clone_state.vertices;

    assert(!(tracer_object_ && !vertices_));

    if(!tracer_object_ && !vertices_)
        init_as_cube();
    else if(!tracer_object_ && vertices_)
    {
        assert(vertices_);
        do_update_tracer_object();
    }

    connect(filename_browse, &QPushButton::clicked, [=]
    {
        load_from_file();
    });
}

ResourceWidget<tracer::Geometry> *TriangleBVHWidget::clone()
{
    CloneState clone_state;
    clone_state.filename      = filename_;
    clone_state.vertices      = vertices_;
    clone_state.tracer_object = tracer_object_;
    return new TriangleBVHWidget(clone_state);
}

void TriangleBVHWidget::save_asset(AssetSaver &saver)
{
    saver.write_string(filename_);
    saver.write(uint32_t(vertices_->size()));
    saver.write_raw(vertices_->data(), sizeof(Vertex) * vertices_->size());
}

void TriangleBVHWidget::load_asset(AssetLoader &loader)
{
    filename_ = loader.read_string();
    filename_label_->setText(filename_);
    filename_label_->setToolTip(filename_);

    vertices_->resize(loader.read<uint32_t>());
    loader.read_raw(vertices_->data(), sizeof(Vertex) * vertices_->size());

    do_update_tracer_object();
}

RC<tracer::ConfigNode> TriangleBVHWidget::to_config(JSONExportContext &ctx) const
{
    auto grp = newRC<tracer::ConfigGroup>();
    grp->insert_str("type", "triangle_bvh");
    grp->insert_child("transform", newRC<tracer::ConfigArray>());

    const auto [ref_filename, filename] = ctx.gen_filename(".bm");

    tracer::factory::save_bin_mesh(
        filename, vertices_->data(), vertices_->size() / 3);

    grp->insert_str("filename", ref_filename);

    return grp;
}

std::vector<EntityInterface::Vertex> TriangleBVHWidget::get_vertices() const
{
    assert(vertices_);
    std::vector<EntityInterface::Vertex> ret;
    for(auto &v : *vertices_)
        ret.push_back({ v.position, v.normal });
    return ret;
}

void TriangleBVHWidget::update_tracer_object_impl()
{
    do_update_tracer_object();
}

void TriangleBVHWidget::init_as_cube()
{
    vertices_ = newRC<std::vector<Vertex>>();

    // +x

    vertices_->push_back({ { +1, -1, -1 }, { 1, 0, 0 }, { 0, 1 } });
    vertices_->push_back({ { +1, +1, -1 }, { 1, 0, 0 }, { 0, 0 } });
    vertices_->push_back({ { +1, +1, +1 }, { 1, 0, 0 }, { 1, 0 } });
    vertices_->push_back({ { +1, -1, -1 }, { 1, 0, 0 }, { 0, 1 } });
    vertices_->push_back({ { +1, +1, +1 }, { 1, 0, 0 }, { 1, 0 } });
    vertices_->push_back({ { +1, -1, +1 }, { 1, 0, 0 }, { 1, 1 } });

    // -x

    vertices_->push_back({ { -1, -1, +1 }, { -1, 0, 0 }, { 0, 1 } });
    vertices_->push_back({ { -1, +1, +1 }, { -1, 0, 0 }, { 0, 0 } });
    vertices_->push_back({ { -1, +1, -1 }, { -1, 0, 0 }, { 1, 0 } });
    vertices_->push_back({ { -1, -1, +1 }, { -1, 0, 0 }, { 0, 1 } });
    vertices_->push_back({ { -1, +1, -1 }, { -1, 0, 0 }, { 1, 0 } });
    vertices_->push_back({ { -1, -1, -1 }, { -1, 0, 0 }, { 1, 1 } });

    // +y

    vertices_->push_back({ { -1, +1, -1 }, { 0, 1, 0 }, { 0, 1 } });
    vertices_->push_back({ { -1, +1, +1 }, { 0, 1, 0 }, { 0, 0 } });
    vertices_->push_back({ { +1, +1, +1 }, { 0, 1, 0 }, { 1, 0 } });
    vertices_->push_back({ { -1, +1, -1 }, { 0, 1, 0 }, { 0, 1 } });
    vertices_->push_back({ { +1, +1, +1 }, { 0, 1, 0 }, { 1, 0 } });
    vertices_->push_back({ { +1, +1, -1 }, { 0, 1, 0 }, { 1, 1 } });

    // -y

    vertices_->push_back({ { +1, -1, -1 }, { 0, -1, 0 }, { 0, 1 } });
    vertices_->push_back({ { +1, -1, +1 }, { 0, -1, 0 }, { 0, 0 } });
    vertices_->push_back({ { -1, -1, +1 }, { 0, -1, 0 }, { 1, 0 } });
    vertices_->push_back({ { +1, -1, -1 }, { 0, -1, 0 }, { 0, 1 } });
    vertices_->push_back({ { -1, -1, +1 }, { 0, -1, 0 }, { 1, 0 } });
    vertices_->push_back({ { -1, -1, -1 }, { 0, -1, 0 }, { 1, 1 } });

    // +z

    vertices_->push_back({ { +1, -1, +1 }, { 0, 0, 1 }, { 0, 1 } });
    vertices_->push_back({ { +1, +1, +1 }, { 0, 0, 1 }, { 0, 0 } });
    vertices_->push_back({ { -1, +1, +1 }, { 0, 0, 1 }, { 1, 0 } });
    vertices_->push_back({ { +1, -1, +1 }, { 0, 0, 1 }, { 0, 1 } });
    vertices_->push_back({ { -1, +1, +1 }, { 0, 0, 1 }, { 1, 0 } });
    vertices_->push_back({ { -1, -1, +1 }, { 0, 0, 1 }, { 1, 1 } });

    // -z

    vertices_->push_back({ { -1, -1, -1 }, { 0, 0, -1 }, { 0, 1 } });
    vertices_->push_back({ { -1, +1, -1 }, { 0, 0, -1 }, { 0, 0 } });
    vertices_->push_back({ { +1, +1, -1 }, { 0, 0, -1 }, { 1, 0 } });
    vertices_->push_back({ { -1, -1, -1 }, { 0, 0, -1 }, { 0, 1 } });
    vertices_->push_back({ { +1, +1, -1 }, { 0, 0, -1 }, { 1, 0 } });
    vertices_->push_back({ { +1, -1, -1 }, { 0, 0, -1 }, { 1, 1 } });

    do_update_tracer_object();

    set_geometry_vertices_dirty();
}

void TriangleBVHWidget::load_from_file()
{
    const QString filename = QFileDialog::getOpenFileName(this, "Filename");
    if(filename.isEmpty())
        return;

    try
    {
        const auto triangles = mesh::load_from_file(filename.toStdString());
        vertices_ = newRC<std::vector<Vertex>>();

        for(auto &tri : triangles)
        {
            vertices_->push_back(tri.vertices[0]);
            vertices_->push_back(tri.vertices[1]);
            vertices_->push_back(tri.vertices[2]);
        }
    }
    catch(...)
    {
        QMessageBox::information(
            this, "Error", "Failed to load mesh from " + filename);
        return;
    }

    filename_ = filename;
    filename_label_->setText(filename_);
    filename_label_->setToolTip(filename_);

    set_dirty_flag();
    set_geometry_vertices_dirty();
}

void TriangleBVHWidget::do_update_tracer_object()
{
    assert(vertices_ && !vertices_->empty());

    std::vector<mesh::triangle_t> triangles;
    for(size_t i = 0; i < vertices_->size(); i += 3)
    {
        triangles.push_back({
            {
                (*vertices_)[i + 0],
                (*vertices_)[i + 1],
                (*vertices_)[i + 2]
            }
        });
    }

    tracer_object_ = tracer::create_triangle_bvh(
        std::move(triangles), {});
}

ResourceWidget<tracer::Geometry> *TriangleBVHWidgetCreator::create_widget(
    ObjectContext &obj_ctx) const
{
    return new TriangleBVHWidget({});
}

AGZ_EDITOR_END
