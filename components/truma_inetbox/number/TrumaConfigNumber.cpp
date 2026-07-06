#include "TrumaConfigNumber.h"

#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG =
    "truma_inetbox.config_number";


void TrumaConfigNumber::setup() {
  this->parent_->get_config()->add_on_message_callback(
      [this](const StatusFrameConfig *status_config) {

        switch (this->type_) {
          case TRUMA_NUMBER_TYPE::CONFIG_TEMP_OFFSET:
            this->publish_state(
                temp_code_to_decimal(
                    status_config->temp_offset));
            break;

          default:
            break;
        }
      });
}


void TrumaConfigNumber::control(float value) {
  switch (this->type_) {
    case TRUMA_NUMBER_TYPE::CONFIG_TEMP_OFFSET:
      this->parent_
          ->get_config()
          ->action_set_temp_offset(value);
      break;

    default:
      break;
  }
}


void TrumaConfigNumber::dump_config() {
  LOG_NUMBER(
      "",
      "Truma Config Number",
      this);

  ESP_LOGCONFIG(
      TAG,
      "  Type '%s'",
      enum_to_c_str(this->type_));
}

}  // namespace truma_inetbox
}  // namespace esphome