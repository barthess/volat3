#ifndef DMFACE_H
#define DMFACE_H

#include <QWidget>

namespace Ui {
class DmFace;
}

class DmFace : public QWidget
{
    Q_OBJECT
    
public:
    explicit DmFace(QWidget *parent = 0);
    ~DmFace();

protected slots:
    void mySlot();
    
private:
    Ui::DmFace *ui;
};

#endif // DMFACE_H
