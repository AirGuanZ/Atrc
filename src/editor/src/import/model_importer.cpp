#include <QFileDialog>
#include <QGridLayout>

#include <agz/editor/import/model_importer.h>
#include <agz/editor/scene/scene_mgr.h>

AGZ_EDITOR_BEGIN

ModelImporter::ModelImporter(QWidget *parent, SceneManager *scene_mgr)
    : QDialog(parent), scene_mgr_(scene_mgr)
{
    browse_filename_    = new QPushButton("Browse", this);
    filename_displayer_ = new ElidedLabel("", this);

    ok_     = new QPushButton("Ok", this);
    cancel_ = new QPushButton("Cancel", this);

    connect(browse_filename_, &QPushButton::clicked, [=]
    {
        const QString filename = QFileDialog::getOpenFileName(this, "Select Obj");
        filename_displayer_->setText(filename);
        filename_displayer_->setToolTip(filename);
    });

    connect(cancel_, &QPushButton::clicked, [=]
    {
        close();
    });

    connect(ok_, &QPushButton::clicked, [=]
    {
        std::vector<mesh::mesh_t> meshes;
        try
        {
            meshes = mesh::load_meshes_from_obj(
                filename_displayer_->text().toStdString());
        }
        catch(const std::exception &err)
        {
            QMessageBox::information(this, "Error", err.what());
            return;
        }

        scene_mgr_->add_meshes(meshes);
        close();
    });

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(browse_filename_, 0, 0);
    layout->addWidget(filename_displayer_, 0, 1);
    layout->addWidget(ok_, 1, 0);
    layout->addWidget(cancel_, 1, 1);
}

AGZ_EDITOR_END
