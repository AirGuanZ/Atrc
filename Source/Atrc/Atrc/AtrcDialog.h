#pragma once

#include <QComboBox>
#include <QListView>

#include "ui_AtrcDialog.h"

class AtrcDialog : public QDialog
{
    Q_OBJECT

public:

    AtrcDialog()
    {
        ui_.setupUi(this);

        comboBox_.setView(new QListView);
        comboBox_.addItem("a", 0);
        comboBox_.addItem("b", 1);
        ui_.layout->addWidget(&comboBox_, 0, 0);

        connect(ui_.closeDialog, &QPushButton::clicked, this, &QDialog::accept);
    }

private:

    Ui::Dialog ui_;
    QComboBox comboBox_;
};
