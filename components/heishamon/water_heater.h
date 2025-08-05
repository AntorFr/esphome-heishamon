#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

class HeishamonComponent;

class HeishamonWaterHeater : public climate::Climate, public Component {
 public:
  void setup() override;
  void dump_config() override;
  
  // Climate interface implementation
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;
  
  // HeishaMon integration
  void set_parent(HeishamonComponent *parent) { parent_ = parent; }
  
  // Update methods called by HeishamonComponent
  void update_target_temperature(float temperature);
  void update_current_temperature(float temperature);
  void update_dhw_state(bool heating);
  void update_dhw_mode(int mode);

 protected:
  HeishamonComponent *parent_{nullptr};
  
  // DHW state tracking
  float target_temperature_{45.0f};
  bool dhw_heating_{false};
  int dhw_mode_{0};  // 0=Off, 1=Heat, 2=Auto
  
  // Private methods
  void send_target_temperature_();
  void send_dhw_mode_();
};

}  // namespace heishamon
}  // namespace esphome
