#pragma once
#include "esphome/core/defines.h"
#ifdef USE_WATER_HEATER

#include "esphome/core/component.h"
#include "esphome/components/water_heater/water_heater.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

class HeishamonComponent;

// DHW (Domestic Hot Water) using native ESPHome WaterHeater platform
// Maps Panasonic DHW modes to WaterHeater modes:
//   WATER_HEATER_MODE_OFF         -> DHW disabled
//   WATER_HEATER_MODE_HEAT_PUMP   -> Normal mode (heat pump only)
//   WATER_HEATER_MODE_ECO         -> Eco mode (energy saving)
//   WATER_HEATER_MODE_PERFORMANCE -> Powerful/Performance mode
class HeishamonWaterHeater : public water_heater::WaterHeater {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // WaterHeater interface implementation
  void control(const water_heater::WaterHeaterCall &call) override;
  water_heater::WaterHeaterCallInternal make_call() override;

  // HeishaMon integration
  void set_parent(HeishamonComponent *parent) { parent_ = parent; }

  // Update methods called by HeishamonComponent
  void update_from_heishamon();

 protected:
  water_heater::WaterHeaterTraits traits() override;

  HeishamonComponent *parent_{nullptr};

  // DHW state tracking
  float target_temperature_internal_{45.0f};
  bool dhw_heating_{false};

  // Private methods
  void send_target_temperature_();
  void send_mode_command_(water_heater::WaterHeaterMode mode);

  // Conversion helpers
  static water_heater::WaterHeaterMode dhw_mode_to_water_heater_mode_(int dhw_mode);
  static int water_heater_mode_to_dhw_mode_(water_heater::WaterHeaterMode mode);
};

}  // namespace heishamon
}  // namespace esphome

#endif  // USE_WATER_HEATER
