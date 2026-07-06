#pragma once

#include <set>

#include "esphome/components/climate/climate.h"
#include "esphome/core/component.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

class TrumaAirconClimate : public Component,
                           public climate::Climate,
                           public Parented<TrumaiNetBoxApp> {
 public:
  void loop() override {}
  void dump_config() override;
  void control(const climate::ClimateCall &call) override;
  void setup() override;
  climate::ClimateTraits traits() override;

  void set_visual_min_temperature(float value) {
    this->visual_min_temperature_ = value;
  }

  void set_visual_max_temperature(float value) {
    this->visual_max_temperature_ = value;
  }

  void set_visual_temperature_step(float value) {
    this->visual_temperature_step_ = value;
  }

  void set_supported_modes(const std::set<climate::ClimateMode> &modes);

 protected:
  std::set<climate::ClimateMode> supported_modes_;
  float visual_min_temperature_{5.0f};
  float visual_max_temperature_{30.0f};
  float visual_temperature_step_{0.5f};
  float temperature_offset_{0.0f};
};

}  // namespace truma_inetbox
}  // namespace esphome