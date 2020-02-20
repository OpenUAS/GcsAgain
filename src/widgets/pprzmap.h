#ifndef PPRZMAP_H
#define PPRZMAP_H

#include <QWidget>
#include <QKeyEvent>
#include "mapitem.h"

namespace Ui {
class PprzMap;
}

class PprzMap : public QWidget
{
    Q_OBJECT

public:
    explicit PprzMap(QWidget *parent = nullptr);
    ~PprzMap();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

protected slots:

private:
    Ui::PprzMap *ui;
    int drawState;

    QList<MapItem*> items;

};

#endif // PPRZMAP_H
