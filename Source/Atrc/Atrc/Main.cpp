#include <QApplication>

#include "Atrc.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Atrc atrc;
    atrc.setWindowTitle("Atrc");
    atrc.show();

    return QApplication::exec();
}
