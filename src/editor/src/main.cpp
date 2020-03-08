#include <QFontDatabase>

#include <agz/editor/editor.h>

#include "./ui/style/DarkStyle.h"

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

#ifdef USE_EMBREE
    agz::tracer::init_embree_device();
    AGZ_SCOPE_GUARD({ agz::tracer::destroy_embree_device(); });
#endif

    QApplication app(argc, argv);

    //app.setStyle(new DarkStyle);
    
    const int font_id = QFontDatabase::addApplicationFont(":/darkstyle/Ubuntu-R.ttf");
    QFont font(QFontDatabase::applicationFontFamilies(font_id).at(0));
    font.setPointSize(9);
    QApplication::setFont(font);

    agz::editor::Editor editor;
    editor.setWindowTitle("Atrc Scene Editor");
    editor.resize(1440, 768);
    editor.showMaximized();

    return QApplication::exec();
}
