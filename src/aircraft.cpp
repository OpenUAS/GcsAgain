#include "aircraft.h"
#include "pprz_dispatcher.h"
#include "dispatcher_ui.h"
#include <QDebug>

Aircraft::Aircraft(QString id, QColor color, QString icon, QString n, FlightPlan fp, shared_ptr<SettingMenu> setting_menu, Airframe air):
    ac_id(id), color(color), icon(icon), _name(n), flight_plan(fp), setting_menu(setting_menu), airframe(air), position(Point2DLatLon(0,0))
{
    status = new AircraftStatus(ac_id);
}

Aircraft::Aircraft() : position(Point2DLatLon(0,0))
{

}

Aircraft::~Aircraft()
{
    if(status != nullptr) {
        //status->deleteLater();
    }
}

void Aircraft::setSetting(shared_ptr<Setting> setting, float value) {

    if(value < setting->getMin() || value > setting->getMax()) {
        qDebug() << "Warning: send setting value " << value << ", out of bounds for setting " << setting->getName().c_str();
    }

    pprzlink::Message dlSetting(PprzDispatcher::get()->getDict()->getDefinition("DL_SETTING"));
    dlSetting.addField("ac_id", ac_id.toStdString());
    dlSetting.addField("index", setting->getNo());
    dlSetting.addField("value", value);
    PprzDispatcher::get()->sendMessage(dlSetting);
}

void Aircraft::setSetting(uint8_t setting_no, float value) {
    for(auto set: setting_menu->getAllSettings()) {
        if(set->getNo() == setting_no) {
            setSetting(set, value);
            break;
        }
    }
}
