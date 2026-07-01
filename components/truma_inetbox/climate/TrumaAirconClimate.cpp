#include "TrumaAirconClimate.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_climate";
void TrumaAirconClimate::setup() {
  ESP_LOGI(TAG, "===== AIRCON SETUP CALLED =====");
  this->parent_->get_aircon_manual()->add_on_message_callback([this](const StatusFrameAirconManual *status_aircon) {
    const uint8_t *p = reinterpret_cast<const uint8_t *>(status_aircon);

    const uint16_t target_raw =
    (static_cast<uint16_t>(p[5]) << 8) | p[4];

    const uint16_t current_raw =
    (static_cast<uint16_t>(p[9]) << 8) | p[8];

    // Publish updated state
    if (target_raw == 0) {
      this->target_temperature = NAN;
    } else {
      this->target_temperature = (target_raw / 10.0f) - 273.0f;
    }
    this->current_temperature = (current_raw / 10.0f) - 273.0f;

    switch (p[0]) {
      case 0x00:
        this->mode = climate::CLIMATE_MODE_OFF;
        break;
      case 0x04:
        this->mode = climate::CLIMATE_MODE_FAN_ONLY;
        break;
      case 0x05:
        this->mode = climate::CLIMATE_MODE_COOL;
        break;
      case 0x06:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      case 0x07:
        this->mode = climate::CLIMATE_MODE_HEAT_COOL;
        break;
      default:
        this->mode = climate::CLIMATE_MODE_OFF;
        break;
    }

    switch (p[2]) {
      case 0x71:
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
      case 0x72:
        this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
        break;
      case 0x73:
        this->fan_mode = climate::CLIMATE_FAN_HIGH;
        break;
      case 0x74:
        this->fan_mode = climate::CLIMATE_FAN_QUIET;
        break;
      case 0x77:
        this->fan_mode = climate::CLIMATE_FAN_AUTO;
        break;
      default:
        this->fan_mode = climate::CLIMATE_FAN_LOW;
        break;
      }

    // switch (status_heater->heating_mode) {
    //   case HeatingMode::HEATING_MODE_ECO:
    //     this->preset = climate::CLIMATE_PRESET_ECO;
    //     break;
    //   case HeatingMode::HEATING_MODE_HIGH:
    //     this->preset = climate::CLIMATE_PRESET_COMFORT;
    //     break;
    //   case HeatingMode::HEATING_MODE_BOOST:
    //     this->preset = climate::CLIMATE_PRESET_BOOST;
    //     break;
    //   default:
    //     this->preset = climate::CLIMATE_PRESET_NONE;
    //     break;
    // }
    this->publish_state();
  });
}

void TrumaAirconClimate::dump_config() { LOG_CLIMATE(TAG, "Truma Aircon Climate", this); }

void TrumaAirconClimate::control(const climate::ClimateCall &call) {
  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();

    if (temp < 16) {
      temp = 16;
    }
    if (temp > 30) {
      temp = 30;
    }

    this->parent_->get_aircon_manual()->action_set_temp(static_cast<uint8_t>(temp));
  }
}

climate::ClimateTraits TrumaAirconClimate::traits() {
  // The capabilities of the climate device
  auto traits = climate::ClimateTraits();
  //traits.set_supports_current_temperature(true);
  traits.add_feature_flags(esphome::climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  
  traits.set_supported_modes({
    climate::CLIMATE_MODE_OFF,
    climate::CLIMATE_MODE_FAN_ONLY,
    climate::CLIMATE_MODE_COOL,
    climate::CLIMATE_MODE_HEAT,
    climate::CLIMATE_MODE_HEAT_COOL,
  });
  
  traits.set_supported_fan_modes({{
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
      climate::CLIMATE_FAN_QUIET,
      climate::CLIMATE_FAN_AUTO,
  }});

  // traits.set_supported_presets({{
  //     climate::CLIMATE_PRESET_NONE,
  //     climate::CLIMATE_PRESET_ECO,
  //     climate::CLIMATE_PRESET_COMFORT,
  //     climate::CLIMATE_PRESET_BOOST,
  // }});
  traits.set_visual_min_temperature(16);
  traits.set_visual_max_temperature(30);
  traits.set_visual_temperature_step(1);
  return traits;
}
  
void TrumaAirconClimate::set_supported_modes(const std::set<climate::ClimateMode> &modes) {
  this->supported_modes_ = modes;
}

}  // namespace truma_inetbox
}  // namespace esphome
