#pragma once

#include "ui_AtrcDialog.h"

class AtrcDialog : public QDialog
{
    Q_OBJECT

public:

    AtrcDialog()
    {
        ui_.setupUi(this);
        connect(ui_.closeDialog, &QPushButton::clicked, this, &QDialog::accept);
    }

private:

    Ui::Dialog ui_;
};
