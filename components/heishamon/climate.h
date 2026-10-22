#pragma once
#include "esphome/core/defines.h"
#ifdef USE_CLIMATE

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

  // Whether the target applies to the cooling side (otherwise heating)
  bool targets_cooling_() const;
  // Whether the request temperature is a shift on the compensation curve
  bool uses_curve_shift_() const;
  // Curve water temperature for the current outdoor temperature, rounded (NAN if unknown)
  float curve_base_() const;
  // Target as shown to the user: the actual water (or room) temperature aimed at
  float compute_target_() const;
  void send_target_(float target);
};

}  // namespace heishamon
}  // namespace esphome
#endif
