#ifndef MATRIXBUTTON_H
#define MATRIXBUTTON_H

#include <QPushButton>
#include <QProcess>

class MatrixButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MatrixButton(const QString &pixpath, const QString& cmd, QWidget *parent = 0);

signals:

public slots:

private:
    QString cmd;

private slots:
    void launch(void);
};

#endif // MATRIXBUTTON_H
