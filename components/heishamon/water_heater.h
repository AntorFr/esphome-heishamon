#pragma once
#include "esphome/core/defines.h"
// Using USE_CLIMATE until ESPHome supports water_heater natively
// PR in progress: https://github.com/esphome/esphome/pull/XXXX
#ifdef USE_CLIMATE

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

class HeishamonComponent;

// DHW (Domestic Hot Water) implemented as Climate until ESPHome supports water_heater
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
  void update_from_heishamon();

 protected:
  HeishamonComponent *parent_{nullptr};
  
  // DHW state tracking
  float target_temperature_{45.0f};
  bool dhw_heating_{false};
  int dhw_mode_{0};  // 0=Off, 1=On, 2=Auto (schedule)
  
  // Private methods
  void send_target_temperature_();
  void send_dhw_mode_();
};

}  // namespace heishamon
}  // namespace esphome
#endif
