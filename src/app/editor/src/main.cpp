#include <QApplication>

#include <agz/editor/editor.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    agz::editor::Editor editor;
    editor.setWindowTitle("Atrc Scene Editor");
    editor.resize(900, 600);
    editor.show();

    app.exec();
}
