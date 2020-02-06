#ifndef POINT2DTILE_H
#define POINT2DTILE_H

#include "utils.h"
#include "QString"

class Point2DLatLon;

class Point2DTile
{

public:
    Point2DTile(double x, double y, int zoom, int zoomMin=0, int zoomMax=19);
    Point2DTile(Point2DLatLon latLon);

    double x() {return xp;}
    double y() {return yp;}
    int xi() {return static_cast<int>(xp);}
    int yi() {return static_cast<int>(yp);}

    int zoom() {return zoomp;}
    void setX(double x) {xp = x;}
    void setY(double y) {yp = y;}
    void setZoom(int zoom) { zoomp = clamp(zoom, zoomMinp, zoomMaxp); }

    bool isValid();

    int zoomMin() {return zoomMinp;}
    int zoomMax() {return zoomMaxp;}
    void setZoomMin(int zoomMin) {zoomMinp = zoomMin;}
    void setZoomMax(int zoomMax) {zoomMaxp = zoomMax;}

    QString to_istring();

private:
    double xp;
    double yp;
    int zoomp;

    int zoomMinp;
    int zoomMaxp;

};


#endif // POINT2DTILE_H