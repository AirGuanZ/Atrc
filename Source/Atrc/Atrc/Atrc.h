#pragma once

#include <QMainWindow>

#include "ui_Atrc.h"

#include "FilmFilter/Box.h"
#include "UI/ResourceSlot.h"

class Atrc : public QMainWindow
{
    Q_OBJECT

public:

    Atrc()
    {
        ui_.setupUi(this);
        ui_.ConsoleDock->setTitleBarWidget(new QWidget);
        ui_.LeftDockWindow->setTitleBarWidget(new QWidget);
        ui_.RightDock->setTitleBarWidget(new QWidget);
        ui_.ConsoleText->document()->setMaximumBlockCount(300);

        textEdit_ = new QTextEdit(ui_.RightDock);
        button_ = new QPushButton(ui_.RightDock);

        connect(ui_.MenuItemQuit, &QAction::triggered, this, &QMainWindow::close);

        connect(ui_.AddMsg, &QPushButton::clicked, this, &Atrc::OnAddMsgClicked);
        connect(ui_.AddErr, &QPushButton::clicked, this, &Atrc::OnAddErrClicked);

        connect(ui_.ClearConsole, &QPushButton::clicked, ui_.ConsoleText, &QTextEdit::clear);

        showText_ = true;
        textEdit_->setText("Minecraft");
        button_->setText("Button");

        textEdit_->show();
        button_->hide();

        ui_.ChangableLayout->addWidget(textEdit_);
        connect(ui_.ChangeRight, &QPushButton::clicked, this, &Atrc::OnChangeRightClicked);

        RegisterBuiltinFilmFilterCreators(filterCreatorMgr_);
        filterSlot_ = std::make_unique<ResourceSlot<FilmFilterInstance>>(filterCreatorMgr_);
        ui_.ChangableLayout->addWidget(filterSlot_->GetWidget());
    }

    void ShowNormalMessage(const std::string &msg)
    {
        ui_.ConsoleText->setTextColor(QColor::fromRgb(0, 0, 0));
        ui_.ConsoleText->append(QString::fromStdString(msg));
    }
    
    void ShowErrorMessage(const std::string &msg)
    {
        ui_.ConsoleText->setTextColor(QColor::fromRgb(255, 0, 0));
        ui_.ConsoleText->append(QString::fromStdString(msg));
    }
    
private:

    void OnAddMsgClicked()
    {
        static int cnt = 0;
        ShowNormalMessage("this is a normal mssage: " + std::to_string(cnt++));
    }

    void OnAddErrClicked()
    {
        static int cnt = 0;
        ShowErrorMessage("this is an error message: " + std::to_string(cnt++));
    }

    void OnChangeRightClicked()
    {
        if(showText_)
        {
            ui_.ChangableLayout->removeWidget(textEdit_);
            ui_.ChangableLayout->insertWidget(0, button_);
            textEdit_->hide();
            button_->show();
        }
        else
        {
            ui_.ChangableLayout->removeWidget(button_);
            ui_.ChangableLayout->insertWidget(0, textEdit_);
            button_->hide();
            textEdit_->show();
        }
        showText_ = !showText_;
    }

    Ui::MainWindow ui_;

    bool showText_;
    QTextEdit *textEdit_;
    QPushButton *button_;

    std::unique_ptr<ResourceSlot<FilmFilterInstance>> filterSlot_;
    FilmFilterCreatorManager filterCreatorMgr_;
};
