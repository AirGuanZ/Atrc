#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QPushButton>

#include <agz/editor/ui/utility/elided_label.h>
#include <agz/utility/mesh.h>

AGZ_EDITOR_BEGIN

class SceneManager;

class ModelImporter : public QDialog
{
public:

    ModelImporter(QWidget *parent, SceneManager *scene_mgr);

private:

    std::vector<mesh::mesh_t> load_meshes_from(
        const std::string &filename) const;

    void transform_mesh(mesh::mesh_t &m, int to_pz, int to_px) const;

    void scale_to_unit_cube(std::vector<mesh::mesh_t> &ms) const;

    QComboBox *up_dir_    = nullptr; // which dir is mapped to +z
    QComboBox *front_dir_ = nullptr; // which dir is mapped to +x

    QCheckBox *to_unit_cube_ = nullptr;

    SceneManager *scene_mgr_;

    QPushButton *ok_     = nullptr;
    QPushButton *cancel_ = nullptr;
};

AGZ_EDITOR_END
