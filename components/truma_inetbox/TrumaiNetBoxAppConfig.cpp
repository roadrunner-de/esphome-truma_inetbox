// components/truma_inetbox/TrumaiNetBoxAppConfig.cpp

#include "TrumaiNetBoxAppConfig.h"

#include <cmath>

#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppConfig";

StatusFrameConfigResponse *TrumaiNetBoxAppConfig::update_prepare() {
  if (this->update_status_prepared_ || this->update_status_stale_) {
    return &this->update_status_;
  }

  this->update_status_ = {};
  this->update_status_.display_brightness = this->data_.display_brightness;
  this->update_status_.language = this->data_.language;
  this->update_status_.ac_offset = this->data_.ac_offset;
  this->update_status_.temp_offset = this->data_.temp_offset;
  this->update_status_.temp_units = this->data_.temp_units;
  this->update_status_.unknown_6 = this->data_.unknown_6;
  this->update_status_.unknown_7 = this->data_.unknown_7;
  this->update_status_.unknown_8 = this->data_.unknown_8;

  this->update_status_prepared_ = true;
  return &this->update_status_;
}

void TrumaiNetBoxAppConfig::create_update_data(StatusFrame *response, uint8_t *response_len,
                                               uint8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_CONFIG_RESPONSE, sizeof(StatusFrameConfigResponse),
                            command_counter);

  response->configResponse = this->update_status_;
  status_frame_calculate_checksum(response);

  *response_len = sizeof(StatusFrameHeader) + sizeof(StatusFrameConfigResponse);

  ESP_LOGD(TAG, "Sending config update: temp_offset %.1f °C",
           temp_code_to_decimal(this->update_status_.temp_offset));

  TrumaStausFrameResponseStorage::update_submitted();
}

float TrumaiNetBoxAppConfig::get_temp_offset() const {
  return temp_code_to_decimal(this->data_.temp_offset);
}

bool TrumaiNetBoxAppConfig::action_set_temp_offset(float offset) {
  if (offset < -5.0f) offset = -5.0f;
  if (offset > 0.0f) offset = 0.0f;

  offset = std::round(offset * 2.0f) / 2.0f;

  const uint16_t raw = static_cast<uint16_t>(std::lround((273.0f + offset) * 10.0f));

  auto *update = this->update_prepare();
  update->temp_offset = static_cast<TargetTemp>(raw);

  ESP_LOGD(TAG, "Set temperature offset %.1f °C, raw=%u", offset, raw);

  return true;
}

void TrumaiNetBoxAppConfig::dump_data() const {
  ESP_LOGD(TAG, "StatusFrameConfig Offset: %.1f", this->get_temp_offset());
}

}  // namespace truma_inetbox
}  // namespace esphome