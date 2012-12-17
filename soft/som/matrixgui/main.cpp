#include <QtGui>
#include <QSettings>

#include "matrixgui.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);

    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForTr(codec);

    QSettings *config = new QSettings("matrixgui.cfg", QSettings::NativeFormat);
    MatrixGui gui(config);
    gui.show();

    return app.exec();
}
