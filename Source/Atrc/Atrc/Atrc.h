#pragma once

#include <QMainWindow>

#include "AtrcDialog.h"
#include "ui_Atrc.h"

class Atrc : public QMainWindow
{
    Q_OBJECT

public:

    explicit Atrc(QWidget *parent = nullptr)
    {
        ui_.setupUi(this);
        connect(ui_.menuItem_quit, &QAction::triggered, this, &QMainWindow::close);
        connect(ui_.showDialog, &QPushButton::clicked, this, [&dialog = dialog_]
        {
            dialog = std::make_unique<AtrcDialog>();
            dialog->show();
        });
    }

private slots:

    void onShowDialog()
    {
        auto dialog = new AtrcDialog;
        dialog->show();
    }

private:

    Ui::MainWindow ui_;
    std::unique_ptr<AtrcDialog> dialog_;
};
