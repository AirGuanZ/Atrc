#include <QFileDialog>
#include <QGridLayout>

#include <agz/editor/imexport/model_importer.h>
#include <agz/editor/scene/scene_mgr.h>

AGZ_EDITOR_BEGIN

namespace
{
    int str_to_axis(const QString &str)
    {
        if(str == "+X") return 0;
        if(str == "+Y") return 1;
        if(str == "+Z") return 2;
        if(str == "-X") return 3;
        if(str == "-Y") return 4;
        return 5;
    }

    QString axis_to_str(int axis)
    {
        static const QString strs[] = {
            "+X", "+Y", "+Z",
            "-X", "-Y", "-Z"
        };
        return strs[axis % 6];
    }
}

ModelImporter::ModelImporter(QWidget *parent, SceneManager *scene_mgr)
    : QDialog(parent), scene_mgr_(scene_mgr)
{
    up_dir_             = new QComboBox(this);
    front_dir_          = new QComboBox(this);
    to_unit_cube_       = new QCheckBox("To Unit Cube", this);
    browse_filename_    = new QPushButton("Browse", this);
    filename_displayer_ = new ElidedLabel("", this);
    ok_                 = new QPushButton("Ok", this);
    cancel_             = new QPushButton("Cancel", this);

    up_dir_   ->addItems({ "+X", "+Y", "+Z", "-X", "-Y", "-Z" });
    front_dir_->addItems({ "+X", "+Y", "+Z", "-X", "-Y", "-Z" });

    up_dir_   ->setCurrentText("+Z");
    front_dir_->setCurrentText("+X");

    to_unit_cube_->setChecked(false);

    connect(up_dir_, &QComboBox::currentTextChanged, [=](const QString &up)
    {
        const int up_axis    = str_to_axis(up);
        const int front_axis = str_to_axis(front_dir_->currentText());

        if(up_axis % 3 == front_axis % 3)
        {
            front_dir_->blockSignals(true);
            front_dir_->setCurrentText(axis_to_str(up_axis + 1));
            front_dir_->blockSignals(false);
        }
    });

    connect(front_dir_, &QComboBox::currentTextChanged, [=](const QString &front)
    {
        const int up_axis    = str_to_axis(up_dir_->currentText());
        const int front_axis = str_to_axis(front);

        if(up_axis % 3 == front_axis % 3)
        {
            up_dir_->blockSignals(true);
            up_dir_->setCurrentText(axis_to_str(front_axis + 5));
            up_dir_->blockSignals(false);
        }
    });

    connect(browse_filename_, &QPushButton::clicked, [=]
    {
        const QString filename = QFileDialog::getOpenFileName(this, "Select Obj");
        filename_displayer_->setText(filename);
        filename_displayer_->setToolTip(filename);
    });

    connect(cancel_, &QPushButton::clicked, [=]
    {
        filename_displayer_->setText("");
        close();
    });

    connect(ok_, &QPushButton::clicked, [=]
    {
        std::vector<mesh::mesh_t> meshes;
        try
        {
            meshes = load_meshes_from(filename_displayer_->text().toStdString());

            const int to_pz = str_to_axis(up_dir_->currentText());
            const int to_px = str_to_axis(front_dir_->currentText());
            for(auto &m : meshes)
                transform_mesh(m, to_pz, to_px);

            if(to_unit_cube_->isChecked())
                scale_to_unit_cube(meshes);
        }
        catch(const std::exception &err)
        {
            QMessageBox::information(this, "Error", err.what());
            return;
        }

        filename_displayer_->setText("");
        scene_mgr_->add_meshes(meshes);
        close();
    });

    QGridLayout *layout = new QGridLayout(this);
    int layout_row = 0;

    layout->addWidget(browse_filename_, layout_row, 0, 1, 1);
    layout->addWidget(filename_displayer_, layout_row, 1, 1, 2);

    ++layout_row;
    layout->addWidget(new QLabel("Up Dir", this), layout_row, 0, 1, 1);
    layout->addWidget(up_dir_, layout_row, 1, 1, 2);

    ++layout_row;
    layout->addWidget(new QLabel("Front Dir", this), layout_row, 0, 1, 1);
    layout->addWidget(front_dir_, layout_row, 1, 1, 2);

    ++layout_row;
    layout->addWidget(to_unit_cube_, layout_row, 0, 1, 1);
    layout->addWidget(ok_, layout_row, 1, 1, 1);
    layout->addWidget(cancel_, layout_row, 2, 1, 1);
}

std::vector<mesh::mesh_t> ModelImporter::load_meshes_from(const std::string &filename) const
{
    if(stdstr::ends_with(filename, ".obj"))
        return mesh::load_meshes_from_obj(filename);
    mesh::mesh_t m;
    m.name = "auto";
    m.triangles = mesh::load_from_file(filename);
    return { std::move(m) };
}

void ModelImporter::transform_mesh(mesh::mesh_t &m, int to_pz, int to_px) const
{
    // M * to_pz = z
    // M * to_px = x
    // M * to_py = y

    Vec3 v_to_px, v_to_pz;
    v_to_px[to_px % 3] = 1;
    v_to_pz[to_pz % 3] = 1;
    Vec3 v_to_py = cross(v_to_pz, v_to_px);

    tracer::Coord coord(v_to_px, v_to_py, v_to_pz);
    for(auto &tri : m.triangles)
    {
        for(auto &v : tri.vertices)
        {
            v.position = coord.global_to_local(v.position);
            v.normal   = coord.global_to_local(v.normal);
        }
    }
}

void ModelImporter::scale_to_unit_cube(std::vector<mesh::mesh_t> &ms) const
{
    tracer::AABB bbox;

    for(auto &m : ms)
    {
        for(auto &t : m.triangles)
        {
            for(auto &v : t.vertices)
                bbox |= v.position;
        }
    }

    const real extent = (bbox.high - bbox.low).max_elem();
    if(extent < real(0.001))
        return;

    const real scale_ratio  = 1 / extent;
    const Vec3 trans_offset = -real(0.5) * (bbox.low + bbox.high);

    const tracer::Transform3 t = tracer::Transform3::scale(Vec3(scale_ratio))
                               * tracer::Transform3::translate(trans_offset);
    for(auto &m : ms)
    {
        for(auto &tri : m.triangles)
        {
            for(auto &v : tri.vertices)
            {
                v.position = t.apply_to_point(v.position);
                v.normal   = t.apply_to_normal(v.normal);
            }
        }
    }
}

AGZ_EDITOR_END
