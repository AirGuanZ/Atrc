#pragma once

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

        connect(ui_.MenuItemQuit, &QAction::triggered, this, &QMainWindow::close);

        connect(ui_.AddMsg, &QPushButton::clicked, this, &Atrc::OnAddMsgClicked);
        connect(ui_.AddErr, &QPushButton::clicked, this, &Atrc::OnAddErrClicked);

        connect(ui_.ClearConsole, &QPushButton::clicked, ui_.ConsoleText, &QTextEdit::clear);

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

    Ui::MainWindow ui_;

    std::unique_ptr<ResourceSlot<FilmFilterInstance>> filterSlot_;
    FilmFilterCreatorManager filterCreatorMgr_;
};
