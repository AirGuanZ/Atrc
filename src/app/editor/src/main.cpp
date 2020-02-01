#include <QApplication>

#include <agz/editor/editor.h>

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    QApplication app(argc, argv);

    agz::editor::Editor editor;
    editor.setWindowTitle("Atrc Scene Editor");
    editor.resize(900, 600);
    editor.showMaximized();

    return QApplication::exec();
}
