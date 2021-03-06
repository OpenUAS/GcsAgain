#include "path_item.h"
#include "maputils.h"
#include <QGraphicsScene>
#include "mapwidget.h"
#include "map_item.h"
#include "AircraftManager.h"

PathItem::PathItem(Point2DLatLon start, QString ac_id, qreal z_value, MapWidget* map, double neutral_scale_zoom, QObject *parent) :
    MapItem(ac_id, z_value, map, neutral_scale_zoom, parent),
    line_width(5)
{
    WaypointItem* wpStart = new WaypointItem(start, ac_id, z_value, map, neutral_scale_zoom, parent);
    init(wpStart);
}

PathItem::PathItem(WaypointItem* wpStart, QString ac_id, qreal z_value, MapWidget* map, double neutral_scale_zoom, QObject *parent) :
    MapItem(ac_id, z_value, map, neutral_scale_zoom, parent),
    line_width(5)
{
    assert(wpStart != nullptr);
    init(wpStart);
}

void PathItem::init(WaypointItem* wpStart) {
    waypoints.append(wpStart);

    connect(
        wpStart, &WaypointItem::waypointMoved, this,
        [=](Point2DLatLon newPos) {
            (void) newPos;
            this->updateGraphics();
        }
    );

    connect(
        wpStart, &MapItem::itemGainedHighlight, this,
        [=]() {
            setHighlighted(true);
            emit(itemGainedHighlight());
        }
    );

    map->addItem(this);
}

void PathItem::addPoint(Point2DLatLon pos) {
    WaypointItem* wp = new WaypointItem(pos, ac_id, z_value, map, neutral_scale_zoom, parent());
    addPoint(wp);
}

void PathItem::addPoint(WaypointItem* wp) {
    assert(wp != nullptr);
    WaypointItem* last_wp = waypoints.last();
    waypoints.append(wp);

    Aircraft aircraft = AircraftManager::get()->getAircraft(ac_id);

    QPointF start_pos = scenePoint(last_wp->position(), zoomLevel(map->zoom()), map->tileSize());
    QPointF end_pos = scenePoint(wp->position(), zoomLevel(map->zoom()), map->tileSize());

    GraphicsLine* line = new GraphicsLine(start_pos, end_pos, aircraft.getColor(), line_width, this);

    QList<QColor> color_variants = makeColorVariants(aircraft.getColor());
    line->setColors(color_variants[2]);

    lines.append(line);
    line->setZValue(z_value - 0.5);

    map->scene()->addItem(line);

    connect(
        wp, &WaypointItem::waypointMoved, this,
        [=](Point2DLatLon newPos) {
            (void) newPos;
            this->updateGraphics();
        }
    );

    connect(
        wp, &MapItem::itemGainedHighlight, this,
        [=]() {
            setHighlighted(true);
            emit(itemGainedHighlight());
        }
    );

    connect(
        line, &GraphicsObject::objectGainedHighlight, this,
        [=]() {
            setHighlighted(true);
            emit(itemGainedHighlight());
        }
    );
}

void PathItem::updatePath(Point2DLatLon pos) {
    (void)pos;
    updateGraphics();
}

void PathItem::setHighlighted(bool h) {
    highlighted = h;
    for(auto wp: waypoints) {
        wp->setHighlighted(h);
    }
    for(auto line: lines) {
        line->setHighlighted(h);
    }
}

void PathItem::setForbidHighlight(bool sh) {
    for(auto wp: waypoints) {
        wp->setForbidHighlight(sh);
    }
    for(auto line: lines) {
        line->setForbidHighlight(sh);
    }
}

void PathItem::setEditable(bool ed) {
    for(auto wp: waypoints) {
        wp->setEditable(ed);
    }
    for(auto line: lines) {
        line->setEditable(ed);
    }
}

void PathItem::setZValue(qreal z) {
    z_value = z;
    //waypoints above lines
    for(auto w:waypoints) {
        w->setZValue(z);
    }
    for(auto l:lines) {
        l->setZValue(z-0.5);
    }
}

void PathItem::updateGraphics() {
    assert(waypoints.length() == lines.length() + 1);

    double s = getScale();
(void)s;

    for(int i=0; i<lines.length(); i++) {
        QPointF start_scene_pos = scenePoint(waypoints[i]->position(), zoomLevel(map->zoom()), map->tileSize());
        QPointF end_scene_pos = scenePoint(waypoints[i+1]->position(), zoomLevel(map->zoom()), map->tileSize());
        lines[i]->setLine(start_scene_pos, end_scene_pos);
    }
}

void PathItem::removeFromScene() {
    for(auto l:lines) {
        map->scene()->removeItem(l);
        delete l;
    }
    lines.clear();
}

void PathItem::setLastLineIgnoreEvents(bool ignore) {
    if(lines.length() > 0) {
        lines.last()->setIgnoreEvent(ignore);
    }
}

void PathItem::setLinesIgnoreEvents(bool ignore) {
    for(auto line: lines) {
        line->setIgnoreEvent(ignore);
    }
}

void PathItem::removeLastWaypoint() {
    auto lastLine = lines.takeLast();
    waypoints.removeLast();
    map->scene()->removeItem(lastLine);
    delete lastLine;
}

void PathItem::setStyle(GraphicsLine::Style s) {
    for(auto line: lines) {
        line->setStyle(s);
    }
    if(s == GraphicsLine::Style::CURRENT_NAV) {
        for(auto w:waypoints) {
            w->setStyle(GraphicsObject::Style::CURRENT_NAV);
        }
    }
}
