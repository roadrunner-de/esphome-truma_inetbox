#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppConfig
    : public TrumaStausFrameResponseStorage<
          StatusFrameConfig,
          StatusFrameConfigResponse> {
 public:
  StatusFrameConfigResponse *update_prepare() override;

  void create_update_data(
      StatusFrame *response,
      uint8_t *response_len,
      uint8_t command_counter) override;

  void dump_data() const override;

  bool action_set_temp_offset(float offset);
};

}  // namespace truma_inetbox
}  // namespace esphome