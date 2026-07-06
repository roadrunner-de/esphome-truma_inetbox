#pragma once

#include "esphome/core/log.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

enum class TRUMA_TEXT_SENSOR_TYPE {
  UNKNOWN,
  AIRCON_MODE,
  AIRCON_FAN_MODE,
  DEVICE_0_INDEX,
  DEVICE_1_INDEX,
};

#ifdef ESPHOME_LOG_HAS_CONFIG
static const char *enum_to_c_str(const TRUMA_TEXT_SENSOR_TYPE val) {
  switch (val) {
    case TRUMA_TEXT_SENSOR_TYPE::AIRCON_MODE:
      return "AIRCON_MODE";
    case TRUMA_TEXT_SENSOR_TYPE::AIRCON_FAN_MODE:
      return "AIRCON_FAN_MODE";
    case TRUMA_TEXT_SENSOR_TYPE::DEVICE_0_INDEX:
      return "DEVICE_0_INDEX";
    case TRUMA_TEXT_SENSOR_TYPE::DEVICE_1_INDEX:
      return "DEVICE_1_INDEX";
    default:
      return "";
  }
}
#endif

class TrumaTextSensor : public Component, public text_sensor::TextSensor, public Parented<TrumaiNetBoxApp> {
 public:
  void setup() override;
  void dump_config() override;

  void set_type(TRUMA_TEXT_SENSOR_TYPE val) { this->type_ = val; }

 protected:
  TRUMA_TEXT_SENSOR_TYPE type_{TRUMA_TEXT_SENSOR_TYPE::UNKNOWN};
};

}  // namespace truma_inetbox
}  // namespace esphome