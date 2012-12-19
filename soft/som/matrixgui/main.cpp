#include <QtGui>
#include <QSettings>

#include "matrixgui.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);

    QCoreApplication::setOrganizationName("KamertonUberSoft");
    QCoreApplication::setApplicationName("matrixgui");
    QSettings *config = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                      QCoreApplication::organizationName(),
                                      QCoreApplication::applicationName());

    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForTr(codec);

    MatrixGui gui(config);
    gui.show();

    return app.exec();
}
