#include "TrumaAirconManualNumber.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_manual_number";

void TrumaAirconManualNumber::setup() {
  this->parent_->get_aircon_manual()->add_on_message_callback([this](const StatusFrameAirconManual *status) {
    switch (this->type_) {
      case TRUMA_NUMBER_TYPE::AIRCON_MANUAL_TEMPERATURE: {
        const uint16_t raw_target =
            (static_cast<uint16_t>(status->unknown_08) << 8) |
            status->unknown_07;
        this->publish_state((raw_target / 10.0f) - 273.0f);
        break;
      }
      default:
      break;
    }
      /* ESP_LOGI(TAG,
         "AIRCON RAW mode=%02X op=%02X energy=%02X target=%02X current_ac=%02X current_room=%02X u07=%02X u08=%02X u11=%02X u12=%02X u15=%02X u16=%02X",
         static_cast<uint8_t>(status->mode),
         static_cast<uint8_t>(status->operation),
         static_cast<uint8_t>(status->energy_mix),
         static_cast<uint8_t>(status->target_temp_aircon),
         static_cast<uint8_t>(status->current_temp_aircon),
         static_cast<uint8_t>(status->current_temp_room),
         status->unknown_07,
         status->unknown_08,
         status->unknown_11,
         status->unknown_12,
         status->unknown_15,
         status->unknown_16);
*/
  });
}

void TrumaAirconManualNumber::control(float value) {
  switch (this->type_) {
    case TRUMA_NUMBER_TYPE::AIRCON_MANUAL_TEMPERATURE:
      this->parent_->get_aircon_manual()->action_set_temp(static_cast<uint8_t>(value));
      break;
    default:
      break;
  }
}

void TrumaAirconManualNumber::dump_config() {
  LOG_NUMBER("", "Truma Aircon Manual Number", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}
}  // namespace truma_inetbox
}  // namespace esphome