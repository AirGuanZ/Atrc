#pragma once

#include <memory>

#include <QImage>
#include <QMainWindow>

namespace Ui { class MainWindow; }

class Atrc : public QMainWindow
{
    Q_OBJECT

public:

    explicit Atrc(QWidget *parent = nullptr);

    ~Atrc();

private slots:

    void OnOpenFile();

    void OnCloseFile();

private:

    Ui::MainWindow *ui_;
    std::unique_ptr<QImage> img_;
};
