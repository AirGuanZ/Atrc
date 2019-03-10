#include <QFile>

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include "Atrc.h"
#include "GL.h"

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
        | _CRTDBG_LEAK_CHECK_DF);
#endif

    QApplication app(argc, argv);
    QApplication::setStyle("Fusion");

    Atrc atrc;
    atrc.showMaximized();
    atrc.setWindowTitle("Atrc");
    atrc.show();

    return QApplication::exec();
}
