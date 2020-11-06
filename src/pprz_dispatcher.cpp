#include "pprz_dispatcher.h"
#include <iostream>
#include "AircraftManager.h"
#include "dispatcher_ui.h"
#include <QDebug>
#include <QApplication>

using namespace std;

PprzDispatcher* PprzDispatcher::singleton = nullptr;

Q_DECLARE_METATYPE(pprzlink::Message)

PprzDispatcher::PprzDispatcher(QObject *parent) : QObject (parent), first_msg(false), started(false)
{
    std::string ivy_name = qApp->property("IVY_NAME").toString().toStdString();
    pprzlink_id = qApp->property("PPRZLINK_ID").toString().toStdString();

    dict = std::make_shared<pprzlink::MessageDictionary>(qApp->property("PPRZLINK_MESSAGES").toString().toStdString());
    link = std::make_unique<pprzlink::IvyLink>(*dict, ivy_name, qApp->property("IVY_BUS").toString().toStdString(), true);

    qRegisterMetaType<pprzlink::Message>();

    connect(this, &PprzDispatcher::dl_values, this, &PprzDispatcher::updateSettings);

}

void PprzDispatcher::bindDeftoSignal(std::string const &name, sig_ptr_t sig) {
    link->BindMessage(dict->getDefinition(name),
        [=](std::string ac_id, pprzlink::Message msg) {
            (void)ac_id;
            std::string id;
            msg.getField("ac_id", id);

            if(AircraftManager::get()->aircraftExists(id.c_str())) {
                if(!first_msg) {
                    first_msg = true;
                    emit(DispatcherUi::get()->ac_selected(QString(id.c_str())));
                }
                emit((this->*sig)(msg));
                AircraftManager::get()->getAircraft(id.c_str()).getStatus()->updateMessage(msg);
            }
        }
    );
}

void PprzDispatcher::requestConfig(std::string ac_id) {
    assert(started);
    pprzlink::Message reqConfig(dict->getDefinition("CONFIG_REQ"));
    reqConfig.addField("ac_id", ac_id);
    reqConfig.setSenderId(pprzlink_id);
    link->sendRequest(reqConfig, [=](std::string ai, pprzlink::Message msg) {
        (void)ai;
        std::string ac_id;
        msg.getField("ac_id", ac_id);
        AircraftManager::get()->addAircraft(msg);
        qDebug() << "new AC !";
        emit(DispatcherUi::get()->new_ac_config(ac_id.c_str()));
    });
}

void PprzDispatcher::sendMessage(pprzlink::Message msg) {
    assert(started);
    msg.setSenderId(pprzlink_id);
    link->sendMessage(msg);
}


void PprzDispatcher::start() {

    link->BindMessage(dict->getDefinition("WAYPOINT_MOVED"),
        [=](std::string ac_id, pprzlink::Message msg) {
            (void)ac_id;
            std::string id;
            uint8_t wp_id = 0;
            msg.getField("ac_id", id);
            msg.getField("wp_id", wp_id);
            if(AircraftManager::get()->aircraftExists(id.c_str()) && wp_id != 0) {
                emit(waypoint_moved(msg));
            }
    });

    bindDeftoSignal("AP_STATUS", &PprzDispatcher::ap_status);
    bindDeftoSignal("NAV_STATUS", &PprzDispatcher::nav_status);
    bindDeftoSignal("CIRCLE_STATUS", &PprzDispatcher::circle_status);
    bindDeftoSignal("SEGMENT_STATUS", &PprzDispatcher::segment_status);
    bindDeftoSignal("ENGINE_STATUS", &PprzDispatcher::engine_status);
    bindDeftoSignal("DL_VALUES", &PprzDispatcher::dl_values);
    bindDeftoSignal("TELEMETRY_STATUS", &PprzDispatcher::telemetry_status);
    bindDeftoSignal("FLY_BY_WIRE", &PprzDispatcher::fly_by_wire);
    bindDeftoSignal("FLIGHT_PARAM", &PprzDispatcher::flight_param);
    bindDeftoSignal("SVSINFO", &PprzDispatcher::svsinfo);


    connect(DispatcherUi::get(), &DispatcherUi::move_waypoint,
        [=](shared_ptr<Waypoint> wp, QString ac_id) {
            pprzlink::Message msg(dict->getDefinition("MOVE_WAYPOINT"));
            msg.setSenderId(pprzlink_id);
            msg.addField("ac_id", ac_id.toStdString());
            msg.addField("wp_id", wp->getId());
            msg.addField("lat", wp->getLat());
            msg.addField("long", wp->getLon());
            msg.addField("alt", wp->getAlt());
            link->sendMessage(msg);
        });

    usleep(10000);
    pprzlink::Message msg(dict->getDefinition("AIRCRAFTS_REQ"));
    msg.setSenderId(pprzlink_id);

    started = true;

    link->sendRequest(msg, [=](std::string ac_id, pprzlink::Message msg) {
        (void)ac_id;
        std::string ac_list;
        msg.getField("ac_list", ac_list);
        std::string id;
        std::stringstream ss(ac_list);
        while (std::getline(ss, id, ',')) {
            if(id != "") {
                requestConfig(id);
            }
        }
    });


    link->BindMessage(dict->getDefinition("NEW_AIRCRAFT"),
        [=](std::string ac_id, pprzlink::Message msg) {
            (void)ac_id;
            std::string id;
            msg.getField("ac_id", id);
            requestConfig(id);
        }
    );
}


void PprzDispatcher::updateSettings(pprzlink::Message msg) {
    std::string ac_id;
    std::string values;
    msg.getField("ac_id", ac_id);
    msg.getField("values", values);
    QString id = QString(ac_id.c_str());

    auto ac = AircraftManager::get()->getAircraft(id);

    auto settings = ac.getSettingMenu()->getAllSettings();
    sort(settings.begin(), settings.end(),
        [](shared_ptr<Setting> sl, shared_ptr<Setting> sr) {
                return sl->getNo() < sr->getNo();
    });

    std::stringstream ss(values);
    std::string token;
    size_t i=0;
    while (std::getline(ss, token, ',')) {
        if(token != "?") {
            double s = stod(token);
            assert(settings[i]->getNo() == static_cast<uint8_t>(i));
            float value = static_cast<float>(s);
            settings[i]->setValue(value);
            emit(DispatcherUi::get()->settingUpdated(ac_id.c_str(), settings[i], value));
        }
        i++;
    }
}
