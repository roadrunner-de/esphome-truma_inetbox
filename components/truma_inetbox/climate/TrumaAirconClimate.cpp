#include "TrumaAirconClimate.h"

#include <cmath>

#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_climate";

void TrumaAirconClimate::setup() {
  ESP_LOGI(TAG, "===== AIRCON SETUP CALLED =====");

  this->temperature_offset_ = this->parent_->get_config()->get_temp_offset();
  ESP_LOGI(TAG, "Aircon temperature offset: %.1f °C", this->temperature_offset_);

  this->parent_->get_aircon_manual()->add_on_message_callback(
      [this](const StatusFrameAirconManual *status_aircon) {
        const uint8_t *p = reinterpret_cast<const uint8_t *>(status_aircon);

        const uint16_t target_raw =
            (static_cast<uint16_t>(p[5]) << 8) | p[4];
        const uint16_t current_raw =
            (static_cast<uint16_t>(p[9]) << 8) | p[8];

        this->temperature_offset_ =
            this->parent_->get_config()->get_temp_offset();

        if (target_raw == 0) {
          this->target_temperature = NAN;
        } else {
          const float device_target = (target_raw / 10.0f) - 273.0f;
          this->target_temperature =
              device_target + this->temperature_offset_;
        }

        const float device_current = (current_raw / 10.0f) - 273.0f;
        this->current_temperature =
            device_current + this->temperature_offset_;

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

        if (this->mode == climate::CLIMATE_MODE_OFF) {
          this->fan_mode = climate::CLIMATE_FAN_OFF;
        } else {
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
        }

        this->publish_state();
      });
}

void TrumaAirconClimate::dump_config() {
  LOG_CLIMATE(TAG, "Truma Aircon Climate", this);
  ESP_LOGCONFIG(TAG, "Temperature offset: %.1f °C", this->temperature_offset_);
}

void TrumaAirconClimate::control(const climate::ClimateCall &call) {
  this->temperature_offset_ = this->parent_->get_config()->get_temp_offset();

  float temp = this->target_temperature;

  if (std::isnan(temp)) {
    temp = 22.0f;
  }

  if (call.get_target_temperature().has_value()) {
    temp = *call.get_target_temperature();
  }

  float device_temp = temp - this->temperature_offset_;

  if (device_temp < 16.0f) {
    device_temp = 16.0f;
  }

  if (device_temp > 30.0f) {
    device_temp = 30.0f;
  }

  if (call.get_target_temperature().has_value()) {
    switch (this->mode) {
      case climate::CLIMATE_MODE_HEAT:
        this->parent_->get_aircon_manual()->action_set_mode(
            static_cast<AirconMode>(0x06),
            static_cast<uint8_t>(device_temp),
            AirconOperation::AC_ONLY);
        break;

      case climate::CLIMATE_MODE_HEAT_COOL:
        this->parent_->get_aircon_manual()->action_set_mode(
            static_cast<AirconMode>(0x07),
            static_cast<uint8_t>(device_temp),
            static_cast<AirconOperation>(0x77));
        break;

      case climate::CLIMATE_MODE_FAN_ONLY:
        this->parent_->get_aircon_manual()->action_set_mode(
            AirconMode::AC_VENTILATION,
            static_cast<uint8_t>(device_temp),
            AirconOperation::AC_ONLY);
        break;

      case climate::CLIMATE_MODE_COOL:
      default:
        this->parent_->get_aircon_manual()->action_set_mode(
            AirconMode::AC_COOLING,
            static_cast<uint8_t>(device_temp),
            AirconOperation::AC_ONLY);
        break;
    }
  }

  if (call.get_fan_mode().has_value()) {
    const climate::ClimateFanMode fan = *call.get_fan_mode();
    AirconMode aircon_mode = AirconMode::AC_COOLING;

    switch (this->mode) {
      case climate::CLIMATE_MODE_HEAT:
        aircon_mode = static_cast<AirconMode>(0x06);
        break;
      case climate::CLIMATE_MODE_HEAT_COOL:
        aircon_mode = static_cast<AirconMode>(0x07);
        break;
      case climate::CLIMATE_MODE_FAN_ONLY:
      case climate::CLIMATE_MODE_OFF:
        aircon_mode = AirconMode::AC_VENTILATION;
        break;
      case climate::CLIMATE_MODE_COOL:
      default:
        aircon_mode = AirconMode::AC_COOLING;
        break;
    }

    switch (fan) {
      case climate::CLIMATE_FAN_AUTO:
        if (aircon_mode == static_cast<AirconMode>(0x07)) {
          this->parent_->get_aircon_manual()->action_set_fan(
              aircon_mode, static_cast<uint8_t>(device_temp), 0x77);
        }
        break;

      case climate::CLIMATE_FAN_LOW:
        if (aircon_mode != static_cast<AirconMode>(0x07)) {
          this->parent_->get_aircon_manual()->action_set_fan(
              aircon_mode, static_cast<uint8_t>(device_temp), 0x71);
        }
        break;

      case climate::CLIMATE_FAN_MEDIUM:
        if (aircon_mode != static_cast<AirconMode>(0x07)) {
          this->parent_->get_aircon_manual()->action_set_fan(
              aircon_mode, static_cast<uint8_t>(device_temp), 0x72);
        }
        break;

      case climate::CLIMATE_FAN_HIGH:
        if (aircon_mode != static_cast<AirconMode>(0x07)) {
          this->parent_->get_aircon_manual()->action_set_fan(
              aircon_mode, static_cast<uint8_t>(device_temp), 0x73);
        }
        break;

      case climate::CLIMATE_FAN_QUIET:
        if (aircon_mode == AirconMode::AC_COOLING) {
          this->parent_->get_aircon_manual()->action_set_fan(
              aircon_mode, static_cast<uint8_t>(device_temp), 0x74);
        }
        break;

      case climate::CLIMATE_FAN_OFF:
        this->parent_->get_aircon_manual()->action_set_mode(
            AirconMode::OFF, 0, AirconOperation::AC_ONLY);
        break;

      default:
        break;
    }
  }

  if (call.get_mode().has_value()) {
    const climate::ClimateMode mode = *call.get_mode();

    switch (mode) {
      case climate::CLIMATE_MODE_OFF:
        this->parent_->get_aircon_manual()->action_set_mode(
            AirconMode::OFF, 0);
        ESP_LOGI(TAG, "AIRCON CONTROL: OFF requested");
        break;

      case climate::CLIMATE_MODE_COOL:
        this->parent_->get_aircon_manual()->action_set_mode(
            AirconMode::AC_COOLING,
            static_cast<uint8_t>(device_temp),
            AirconOperation::AC_ONLY);
        break;

      case climate::CLIMATE_MODE_HEAT:
        this->parent_->get_aircon_manual()->action_set_mode(
            static_cast<AirconMode>(0x06),
            static_cast<uint8_t>(device_temp),
            AirconOperation::AC_ONLY);
        break;

      case climate::CLIMATE_MODE_HEAT_COOL:
        this->parent_->get_aircon_manual()->action_set_mode(
            static_cast<AirconMode>(0x07),
            static_cast<uint8_t>(device_temp),
            static_cast<AirconOperation>(0x77));
        break;

      case climate::CLIMATE_MODE_FAN_ONLY:
        this->parent_->get_aircon_manual()->action_set_mode(
            AirconMode::AC_VENTILATION,
            static_cast<uint8_t>(device_temp),
            AirconOperation::AC_ONLY);
        break;

      default:
        break;
    }
  }
}

climate::ClimateTraits TrumaAirconClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.add_feature_flags(
      climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);

  traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_FAN_ONLY,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_HEAT_COOL,
  });

  switch (this->mode) {
    case climate::CLIMATE_MODE_HEAT_COOL:
      traits.set_supported_fan_modes({
          climate::CLIMATE_FAN_AUTO,
      });
      break;

    case climate::CLIMATE_MODE_COOL:
      traits.set_supported_fan_modes({
          climate::CLIMATE_FAN_LOW,
          climate::CLIMATE_FAN_MEDIUM,
          climate::CLIMATE_FAN_HIGH,
          climate::CLIMATE_FAN_QUIET,
      });
      break;

    case climate::CLIMATE_MODE_HEAT:
    case climate::CLIMATE_MODE_FAN_ONLY:
      traits.set_supported_fan_modes({
          climate::CLIMATE_FAN_LOW,
          climate::CLIMATE_FAN_MEDIUM,
          climate::CLIMATE_FAN_HIGH,
      });
      break;

    case climate::CLIMATE_MODE_OFF:
    default:
      traits.set_supported_fan_modes({
          climate::CLIMATE_FAN_OFF,
      });
      break;
  }

  traits.set_visual_min_temperature(16);
  traits.set_visual_max_temperature(30);
  traits.set_visual_temperature_step(1);

  return traits;
}

void TrumaAirconClimate::set_supported_modes(
    const std::set<climate::ClimateMode> &modes) {
  this->supported_modes_ = modes;
}

}  // namespace truma_inetbox
}  // namespace esphome