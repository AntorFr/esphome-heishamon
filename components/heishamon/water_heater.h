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

// DHW operating mode presets
enum class DHWOperatingMode : uint8_t {
  NORMAL = 0,       // Normal/Standard mode
  ECO = 1,          // Eco mode (energy saving)
  POWERFUL = 2,     // Powerful/Performance mode
};

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
  DHWOperatingMode dhw_operating_mode_{DHWOperatingMode::NORMAL};
  bool force_dhw_{false};  // Force DHW active state
  
  // Custom preset names
  static const char* PRESET_NORMAL;
  static const char* PRESET_ECO;
  static const char* PRESET_POWERFUL;
  
  // Private methods
  void send_target_temperature_();
  void send_dhw_operating_mode_();
  
  // Helper to convert mode to preset string
  std::string mode_to_preset_string_(DHWOperatingMode mode);
  DHWOperatingMode preset_string_to_mode_(const std::string &preset);
};

}  // namespace heishamon
}  // namespace esphome
#endif
