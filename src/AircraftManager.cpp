//
// Created by fabien on 22/02/2020.
//

#include "AircraftManager.h"
#include <QDebug>
#include <QApplication>
#include "flightplan.h"
#include "setting_menu.h"
#include "airframe.h"

AircraftManager* AircraftManager::singleton = nullptr;

AircraftManager::AircraftManager() {

}

Aircraft& AircraftManager::getAircraft(QString id) {
    if(aircrafts.find(id) != aircrafts.end()) {
        return aircrafts[id];
    } else {
        throw runtime_error("No such aircraft!");
    }
}

QList<Aircraft> AircraftManager::getAircrafts() {
    return aircrafts.values();
}

void AircraftManager::addAircraft(pprzlink::Message msg) {
    std::string ac_id, ac_name, default_gui_color, flight_plan, airframe, radio, settings;
    msg.getField("ac_id", ac_id);
    msg.getField("ac_name", ac_name);
    msg.getField("default_gui_color", default_gui_color);
    msg.getField("flight_plan", flight_plan);
    msg.getField("airframe", airframe);
    msg.getField("radio", radio);
    msg.getField("settings", settings);

    QColor color = parseColor(default_gui_color);

    QString id = QString::fromStdString(ac_id);

    if(aircraftExists(id)) {
        qDebug() << "Aircraft " << id << " already exits!";
        return;
    }

    FlightPlan fp(flight_plan.c_str());
    shared_ptr<SettingMenu> sm = make_shared<SettingMenu>(settings.c_str());

    Airframe air(airframe.c_str());

    QString icon = qApp->property("PATH_AIRCRAFT_ICON").toString() + "/" + QString(air.getIconName().c_str()) + ".svg";
    qDebug() << "icon: " << icon;

    aircrafts[id] = Aircraft(id, color, icon, QString::fromStdString(ac_name), fp, sm, air);
}

bool AircraftManager::aircraftExists(QString id) {
    if(aircrafts.find(id) != aircrafts.end()) {
        return true;
    } else {
        return false;
    }
}

void AircraftManager::removeAircraft(QString ac_id) {
    (void)ac_id;
    if(aircraftExists(ac_id)) {
        aircrafts.remove(ac_id);
    }
}


QColor AircraftManager::parseColor(std::string str) {
    QColor color = QColor();

    if(str[0] == '#' && str.size() == 13) {
        int r = std::stoi(str.substr(1, 4), nullptr, 16) >> 8;
        int g = std::stoi(str.substr(5, 4), nullptr, 16)  >> 8;
        int b = std::stoi(str.substr(9, 4), nullptr, 16)  >> 8;
        color = QColor(r, g, b);
    } else {
        color = QColor(str.c_str());
    }

    if(!color.isValid()) {
        color = QColor(qApp->property("DEFAULT_COLOR").toString());
    }

    return color;
}
