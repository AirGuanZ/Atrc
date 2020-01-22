#include <QApplication>

#include <agz/gui/gui.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GUI gui;
    gui.setWindowTitle("Atrc Renderer by AirGuanZ");
    gui.resize(640, 480);
    gui.show();

    QApplication::exec();
}
