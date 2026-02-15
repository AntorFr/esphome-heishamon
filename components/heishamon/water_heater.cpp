#include "esphome/core/defines.h"
#ifdef USE_WATER_HEATER
#include "water_heater.h"
#include "heishamon.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.water_heater";

void HeishamonWaterHeater::setup() {
  // Set initial state
  this->mode_ = water_heater::WATER_HEATER_MODE_HEAT_PUMP;  // Default: Normal/Heat pump mode
  this->target_temperature_ = this->target_temperature_internal_;
  this->current_temperature_ = NAN;
  this->set_state_flag_(water_heater::WATER_HEATER_STATE_ON, true);

  ESP_LOGCONFIG(TAG, "HeishaMon DHW (Water Heater) setup complete");
}

void HeishamonWaterHeater::dump_config() {
  LOG_WATER_HEATER("", "HeishaMon DHW", this);
  ESP_LOGCONFIG(TAG, "  Target Temperature: %.1f°C", this->target_temperature_internal_);
  this->dump_traits_(TAG);
}

water_heater::WaterHeaterTraits HeishamonWaterHeater::traits() {
  water_heater::WaterHeaterTraits traits;

  traits.set_supports_current_temperature(true);
  traits.add_feature_flags(water_heater::WATER_HEATER_SUPPORTS_TARGET_TEMPERATURE);
  traits.add_feature_flags(water_heater::WATER_HEATER_SUPPORTS_OPERATION_MODE);
  traits.add_feature_flags(water_heater::WATER_HEATER_SUPPORTS_ON_OFF);

  // Supported modes matching Panasonic DHW modes
  water_heater::WaterHeaterModeMask modes;
  modes.insert(water_heater::WATER_HEATER_MODE_OFF);        // DHW disabled
  modes.insert(water_heater::WATER_HEATER_MODE_HEAT_PUMP);  // Normal mode
  modes.insert(water_heater::WATER_HEATER_MODE_ECO);        // Eco mode
  modes.insert(water_heater::WATER_HEATER_MODE_PERFORMANCE); // Powerful mode
  traits.set_supported_modes(modes);

  // Temperature settings for DHW (40-75°C per HeishaMon protocol)
  traits.set_min_temperature(40.0f);
  traits.set_max_temperature(75.0f);
  traits.set_target_temperature_step(1.0f);

  return traits;
}

water_heater::WaterHeaterCallInternal HeishamonWaterHeater::make_call() {
  return water_heater::WaterHeaterCallInternal(this);
}

void HeishamonWaterHeater::control(const water_heater::WaterHeaterCall &call) {
  bool changed = false;

  // Handle mode changes
  if (call.get_mode().has_value()) {
    water_heater::WaterHeaterMode new_mode = *call.get_mode();
    if (new_mode != this->mode_) {
      this->set_mode_(new_mode);
      changed = true;

      // Update on/off state
      this->set_state_flag_(water_heater::WATER_HEATER_STATE_ON,
                            new_mode != water_heater::WATER_HEATER_MODE_OFF);

      // Send mode command to heat pump
      this->send_mode_command_(new_mode);

      ESP_LOGD(TAG, "DHW mode changed to: %d", static_cast<int>(new_mode));
    }
  }

  // Handle target temperature changes
  if (!std::isnan(call.get_target_temperature())) {
    float new_target = call.get_target_temperature();

    // Validate temperature range (40-75°C)
    if (new_target >= 40.0f && new_target <= 75.0f) {
      if (fabsf(new_target - this->target_temperature_internal_) > 0.5f) {
        this->target_temperature_internal_ = new_target;
        this->set_target_temperature_(new_target);
        changed = true;

        this->send_target_temperature_();
        ESP_LOGD(TAG, "DHW target temperature changed to: %.1f°C", new_target);
      }
    } else {
      ESP_LOGW(TAG, "Invalid DHW temperature: %.1f°C (range: 40-75°C)", new_target);
    }
  }

  if (changed) {
    this->publish_state();
  }
}

void HeishamonWaterHeater::update_from_heishamon() {
  if (this->parent_ == nullptr) return;

  // Store old values to detect changes
  float old_current_temp = this->current_temperature_;
  float old_target_temp = this->target_temperature_;
  bool old_heating = this->dhw_heating_;
  water_heater::WaterHeaterMode old_mode = this->mode_;

  // Get current values from parent
  float new_current_temp = this->parent_->get_dhw_current_temperature();
  float new_target_temp = this->parent_->get_dhw_target_temperature();
  bool new_heating = this->parent_->get_dhw_heating_state();
  int new_dhw_mode = this->parent_->get_dhw_mode();

  bool changed = false;

  // Update current temperature
  if (!std::isnan(new_current_temp)) {
    if (std::isnan(old_current_temp) || fabsf(new_current_temp - old_current_temp) > 0.1f) {
      this->set_current_temperature(new_current_temp);
      changed = true;
    }
  }

  // Update target temperature (only if not being controlled by user)
  if (!std::isnan(new_target_temp)) {
    if (std::isnan(old_target_temp) || fabsf(new_target_temp - old_target_temp) > 0.1f) {
      this->set_target_temperature_(new_target_temp);
      this->target_temperature_internal_ = new_target_temp;
      changed = true;
    }
  }

  // Update mode from heat pump
  water_heater::WaterHeaterMode new_mode = dhw_mode_to_water_heater_mode_(new_dhw_mode);
  if (new_mode != old_mode) {
    this->set_mode_(new_mode);
    this->set_state_flag_(water_heater::WATER_HEATER_STATE_ON,
                          new_mode != water_heater::WATER_HEATER_MODE_OFF);
    changed = true;
    ESP_LOGD(TAG, "DHW mode updated from heat pump: %d (dhw_mode=%d)",
             static_cast<int>(new_mode), new_dhw_mode);
  }

  // Update heating state tracking
  if (new_heating != old_heating) {
    this->dhw_heating_ = new_heating;
    changed = true;
  }

  if (changed) {
    ESP_LOGD(TAG, "DHW state update: mode=%d, current=%.1f, target=%.1f, heating=%s",
             static_cast<int>(this->mode_), this->current_temperature_,
             this->target_temperature_, YESNO(this->dhw_heating_));
    this->publish_state();
  }
}

void HeishamonWaterHeater::send_target_temperature_() {
  if (this->parent_ != nullptr) {
    // Send command to set DHW target temperature (SET11)
    this->parent_->send_number_command("SetDHWTemp", this->target_temperature_internal_);
    ESP_LOGD(TAG, "Sent DHW temperature command: %.1f°C", this->target_temperature_internal_);
  }
}

void HeishamonWaterHeater::send_mode_command_(water_heater::WaterHeaterMode mode) {
  if (this->parent_ == nullptr) return;

  if (mode == water_heater::WATER_HEATER_MODE_OFF) {
    // Disable DHW via force DHW off
    this->parent_->send_command("SetForceDHW", (uint8_t) 0);
    ESP_LOGD(TAG, "DHW disabled - sent SetForceDHW=0");
  } else {
    // Send DHW operating mode
    int dhw_mode = water_heater_mode_to_dhw_mode_(mode);
    this->parent_->send_command("SetDHWMode", (uint8_t) dhw_mode);
    ESP_LOGD(TAG, "Sent DHW operating mode command: %d", dhw_mode);
  }
}

// Convert Panasonic DHW mode (0=Normal, 1=Eco, 2=Powerful) to WaterHeaterMode
water_heater::WaterHeaterMode HeishamonWaterHeater::dhw_mode_to_water_heater_mode_(int dhw_mode) {
  switch (dhw_mode) {
    case 1:
      return water_heater::WATER_HEATER_MODE_ECO;
    case 2:
      return water_heater::WATER_HEATER_MODE_PERFORMANCE;
    case 0:
    default:
      return water_heater::WATER_HEATER_MODE_HEAT_PUMP;  // Normal = Heat Pump mode
  }
}

// Convert WaterHeaterMode to Panasonic DHW mode code
int HeishamonWaterHeater::water_heater_mode_to_dhw_mode_(water_heater::WaterHeaterMode mode) {
  switch (mode) {
    case water_heater::WATER_HEATER_MODE_ECO:
      return 1;
    case water_heater::WATER_HEATER_MODE_PERFORMANCE:
      return 2;
    case water_heater::WATER_HEATER_MODE_HEAT_PUMP:
    default:
      return 0;  // Normal
  }
}

}  // namespace heishamon
}  // namespace esphome

#endif  // USE_WATER_HEATER
