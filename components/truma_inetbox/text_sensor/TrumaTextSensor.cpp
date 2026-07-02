#include "TrumaTextSensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.text_sensor";

static const char *aircon_mode_to_text(uint8_t mode) {
  switch (mode) {
    case 0x00:
      return "OFF";
    case 0x04:
      return "FAN_ONLY";
    case 0x05:
      return "COOL";
    case 0x06:
      return "HEAT";
    case 0x07:
      return "AUTO";
    default:
      return "UNKNOWN";
  }
}

static const char *aircon_fan_mode_to_text(uint8_t fan_mode) {
  switch (fan_mode) {
    case 0x71:
      return "LOW";
    case 0x72:
      return "MEDIUM";
    case 0x73:
      return "HIGH";
    case 0x74:
      return "QUIET";
    case 0x77:
      return "AUTO";
    default:
      return "UNKNOWN";
  }
}

void TrumaTextSensor::setup() {
  this->parent_->get_aircon_manual()->add_on_message_callback([this](const StatusFrameAirconManual *status_aircon) {
    const uint8_t *p = reinterpret_cast<const uint8_t *>(status_aircon);

    switch (this->type_) {
      case TRUMA_TEXT_SENSOR_TYPE::AIRCON_MODE:
        this->publish_state(aircon_mode_to_text(p[0]));
        break;

      case TRUMA_TEXT_SENSOR_TYPE::AIRCON_FAN_MODE:
        if (p[0] == 0x00) {
          this->publish_state("OFF");
        } else {
          this->publish_state(aircon_fan_mode_to_text(p[2]));
        }
        break;

      default:
        break;
    }
  });
}

void TrumaTextSensor::dump_config() {
  LOG_TEXT_SENSOR("", "Truma Text Sensor", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}

}  // namespace truma_inetbox
}  // namespace esphome