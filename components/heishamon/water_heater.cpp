#include "esphome/core/defines.h"
// Using USE_CLIMATE until ESPHome supports water_heater natively
#ifdef USE_CLIMATE
#include "water_heater.h"
#include "heishamon.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.water_heater";

void HeishamonWaterHeater::setup() {
  // Set initial state - DHW is typically always enabled
  this->target_temperature = this->target_temperature_;
  this->current_temperature = NAN;
  this->mode = climate::CLIMATE_MODE_HEAT;  // DHW is normally ON
  this->action = climate::CLIMATE_ACTION_IDLE;
  
  ESP_LOGCONFIG(TAG, "HeishaMon DHW (Water Heater) setup complete");
}

void HeishamonWaterHeater::dump_config() {
  ESP_LOGCONFIG(TAG, "HeishaMon DHW (Water Heater):");
  ESP_LOGCONFIG(TAG, "  Target Temperature: %.1f°C", this->target_temperature_);
}

climate::ClimateTraits HeishamonWaterHeater::traits() {
  auto traits = climate::ClimateTraits();
  
  // Supported features
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_ACTION);
  
  // Supported modes for DHW
  traits.add_supported_mode(climate::CLIMATE_MODE_OFF);   // DHW Off
  traits.add_supported_mode(climate::CLIMATE_MODE_HEAT);  // DHW On (Force DHW)
  
  // Temperature settings for DHW (40-75°C per HeishaMon protocol)
  traits.set_visual_min_temperature(40.0f);
  traits.set_visual_max_temperature(75.0f);
  traits.set_visual_temperature_step(1.0f);
  
  return traits;
}

void HeishamonWaterHeater::control(const climate::ClimateCall &call) {
  bool changed = false;
  
  // Handle mode changes
  if (call.get_mode().has_value()) {
    climate::ClimateMode new_mode = *call.get_mode();
    if (new_mode != this->mode) {
      this->mode = new_mode;
      changed = true;
      
      // Convert to DHW mode and send command
      switch (new_mode) {
        case climate::CLIMATE_MODE_OFF:
          this->dhw_mode_ = 0;  // DHW Off (stop force DHW)
          break;
        case climate::CLIMATE_MODE_HEAT:
          this->dhw_mode_ = 1;  // DHW On (Force DHW)
          break;
        default:
          ESP_LOGW(TAG, "Unsupported mode: %d", (int)new_mode);
          return;
      }
      
      this->send_dhw_mode_();
      ESP_LOGD(TAG, "DHW mode changed to: %d", this->dhw_mode_);
    }
  }
  
  // Handle target temperature changes
  if (call.get_target_temperature().has_value()) {
    float new_target = *call.get_target_temperature();
    
    // Validate temperature range (40-75°C)
    if (new_target >= 40.0f && new_target <= 75.0f) {
      if (fabsf(new_target - this->target_temperature_) > 0.5f) {
        this->target_temperature_ = new_target;
        this->target_temperature = new_target;
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
  float old_current_temp = this->current_temperature;
  float old_target_temp = this->target_temperature;
  bool old_heating = this->dhw_heating_;
  
  // Get current values from parent
  float new_current_temp = this->parent_->get_dhw_current_temperature();
  float new_target_temp = this->parent_->get_dhw_target_temperature();
  bool new_heating = this->parent_->get_dhw_heating_state();
  
  bool changed = false;
  
  // Update current temperature
  if (!std::isnan(new_current_temp)) {
    if (std::isnan(old_current_temp) || fabsf(new_current_temp - old_current_temp) > 0.1f) {
      this->current_temperature = new_current_temp;
      changed = true;
    }
  }
  
  // Update target temperature (only if not being controlled by user)
  if (!std::isnan(new_target_temp)) {
    if (std::isnan(old_target_temp) || fabsf(new_target_temp - old_target_temp) > 0.1f) {
      this->target_temperature = new_target_temp;
      this->target_temperature_ = new_target_temp;
      changed = true;
    }
  }
  
  // Update heating state (action)
  if (new_heating != old_heating) {
    this->dhw_heating_ = new_heating;
    if (new_heating) {
      this->action = climate::CLIMATE_ACTION_HEATING;
      // When actively heating, mode should be HEAT
      if (this->mode != climate::CLIMATE_MODE_HEAT) {
        this->mode = climate::CLIMATE_MODE_HEAT;
      }
    } else {
      this->action = climate::CLIMATE_ACTION_IDLE;
    }
    changed = true;
  }
  
  if (changed) {
    ESP_LOGD(TAG, "DHW state update: mode=%d, action=%d, current=%.1f, target=%.1f",
             this->mode, this->action, this->current_temperature, this->target_temperature);
    this->publish_state();
  }
}

void HeishamonWaterHeater::send_target_temperature_() {
  if (this->parent_ != nullptr) {
    // Send command to set DHW target temperature (SET11)
    this->parent_->send_number_command("SetDHWTemp", this->target_temperature_);
    ESP_LOGD(TAG, "Sent DHW temperature command: %.1f°C", this->target_temperature_);
  }
}

void HeishamonWaterHeater::send_dhw_mode_() {
  if (this->parent_ != nullptr) {
    // Send command to force DHW (SET1)
    // 0 = Off, 1 = Force DHW On
    this->parent_->send_command("SetForceDHW", (uint8_t)this->dhw_mode_);
    ESP_LOGD(TAG, "Sent Force DHW command: %d", this->dhw_mode_);
  }
}

}  // namespace heishamon
}  // namespace esphome
#endif
