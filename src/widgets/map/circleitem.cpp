#include "circleitem.h"
#include <QPen>
#include "math.h"
#include <QApplication>
#include <QDebug>
#include "maputils.h"
#include <QGraphicsScene>

CircleItem::CircleItem(Point2DLatLon pt, double radius, QColor color, double zoom, int tile_size, double neutral_scale_zoom, QObject *parent) :
    MapItem(zoom, tile_size, neutral_scale_zoom, parent),
    _radius(radius), stroke(5)
{
    QPointF scene_pos = scenePoint(pt, zoomLevel(zoom), tile_size);
    center = new WaypointItem(pt, 20, Qt::blue, tile_size, zoom, neutral_scale_zoom, parent);
    center->setZoomFactor(1.1);
    double pixelRadius = distMeters2Tile(radius, pt.lat(), zoomLevel(_zoom)) * tile_size;
    circle = new GraphicsCircle(pixelRadius);
    circle->setPos(scene_pos);
    circle->setPen(QPen(QBrush(color), stroke));
    circle->setZValue(100);

    connect(
        center, &WaypointItem::waypointMoved,
        [=](Point2DLatLon latlon) {
            QPointF p = scenePoint(latlon, zoomLevel(_zoom), tile_size);
            circle->setPos(p);
            emit(circleMoved(latlon));
        }
    );

    connect(
        circle, &GraphicsCircle::circleScaled,
        [=](qreal size) {
            _radius = distTile2Meters(circle->pos().y()/tile_size, size/tile_size, zoomLevel(_zoom));
            emit(circleScaled(_radius));
        }
    );
}

void CircleItem::add_to_scene(QGraphicsScene* scene) {
    center->add_to_scene(scene);
    scene->addItem(circle);
}

void CircleItem::scaleToZoom(double zoom, double viewScale) {
    _zoom = zoom;
    _view_scale = viewScale;
    center->scaleToZoom(zoom, viewScale);

    double pixelRadius = distMeters2Tile(_radius, center->position().lat(), zoomLevel(_zoom))*tile_size;

    QPointF scene_pos = scenePoint(center->position(), zoomLevel(zoom), tile_size);
    circle->setPos(scene_pos);
    circle->setRadius(pixelRadius);

    double s = pow(zoom_factor, zoom - neutral_scale_zoom)/viewScale;
    QPen p = circle->pen();
    p.setWidth(static_cast<int>(stroke * s));
    circle->setPen(p);
}