#include "dmface.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DmFace w;
    w.show();

    return a.exec();
}
