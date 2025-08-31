#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "heishamon.h"

namespace esphome {
namespace heishamon {

class HeishaMonClimate : public climate::Climate, public Component {
 public:
  void set_parent(HeishamonComponent *parent) { parent_ = parent; }
  void set_zone_id(uint8_t zone_id) { zone_id_ = zone_id; }
  void set_supports_heat(bool supports_heat) { supports_heat_ = supports_heat; }
  void set_supports_cool(bool supports_cool) { supports_cool_ = supports_cool; }

  void setup() override;
  void dump_config() override;
  
  // Climate interface
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;

  // Update from parent component
  void update_from_heishamon();

 protected:
  HeishamonComponent *parent_{nullptr};
  uint8_t zone_id_{1};
  bool supports_heat_{true};
  bool supports_cool_{false};

  // Internal state tracking
  float current_temperature_{NAN};
  float target_temperature_{NAN};
  climate::ClimateMode current_mode_{climate::CLIMATE_MODE_OFF};
  
  // Heat pump state tracking
  bool global_heat_enabled_{false};
  bool global_cool_enabled_{false};
  bool zone_heat_enabled_{false};
  bool zone_cool_enabled_{false};
  
  // Helper methods
  void update_operating_mode();
  void update_zone_temperatures();
  void update_zone_states();
  void send_climate_command();
  bool is_zone_active() const;
  climate::ClimateMode get_current_hvac_mode() const;
};

}  // namespace heishamon
}  // namespace esphome
