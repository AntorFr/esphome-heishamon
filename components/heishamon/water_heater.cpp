#include "water_heater.h"
#include "heishamon.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.water_heater";

void HeishamonWaterHeater::setup() {
  // Set initial state
  this->target_temperature = this->target_temperature_;
  this->current_temperature = NAN;
  this->mode = climate::CLIMATE_MODE_OFF;
  this->action = climate::CLIMATE_ACTION_IDLE;
  
  ESP_LOGCONFIG(TAG, "HeishaMon Water Heater setup complete");
}

void HeishamonWaterHeater::dump_config() {
  ESP_LOGCONFIG(TAG, "HeishaMon Water Heater:");
  ESP_LOGCONFIG(TAG, "  Target Temperature: %.1f°C", this->target_temperature_);
}

climate::ClimateTraits HeishamonWaterHeater::traits() {
  auto traits = climate::ClimateTraits();
  
  // Supported modes for DHW
  traits.set_supported_modes({
    climate::CLIMATE_MODE_OFF,     // DHW Off
    climate::CLIMATE_MODE_HEAT,    // DHW On (manual)
    climate::CLIMATE_MODE_AUTO     // DHW Auto (schedule)
  });
  
  // Temperature settings for DHW
  traits.set_supports_current_temperature(true);
  // Note: set_supports_target_temperature method might have changed in ESPHome 2025.8.2
  // traits.set_supports_target_temperature(true);
  traits.set_visual_min_temperature(40.0f);
  traits.set_visual_max_temperature(65.0f);
  traits.set_visual_temperature_step(1.0f);
  
  // DHW doesn't support fan modes, swing, etc.
  // Note: these setter methods might have changed in ESPHome 2025.8.2
  // traits.set_supports_fan_modes(false);
  // traits.set_supports_swing_modes(false);
  // traits.set_supports_presets(false);
  
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
          this->dhw_mode_ = 0;  // DHW Off
          break;
        case climate::CLIMATE_MODE_HEAT:
          this->dhw_mode_ = 1;  // DHW On
          break;
        case climate::CLIMATE_MODE_AUTO:
          this->dhw_mode_ = 2;  // DHW Auto
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
    
    // Validate temperature range
    if (new_target >= 40.0f && new_target <= 65.0f) {
      if (abs(new_target - this->target_temperature_) > 0.5f) {
        this->target_temperature_ = new_target;
        this->target_temperature = new_target;
        changed = true;
        
        this->send_target_temperature_();
        ESP_LOGD(TAG, "DHW target temperature changed to: %.1f°C", new_target);
      }
    } else {
      ESP_LOGW(TAG, "Invalid DHW temperature: %.1f°C (range: 40-65°C)", new_target);
    }
  }
  
  if (changed) {
    this->publish_state();
  }
}

void HeishamonWaterHeater::update_target_temperature(float temperature) {
  if (abs(temperature - this->target_temperature_) > 0.5f) {
    this->target_temperature_ = temperature;
    this->target_temperature = temperature;
    this->publish_state();
    ESP_LOGD(TAG, "DHW target temperature updated: %.1f°C", temperature);
  }
}

void HeishamonWaterHeater::update_current_temperature(float temperature) {
  if (abs(temperature - this->current_temperature) > 0.5f || std::isnan(this->current_temperature)) {
    this->current_temperature = temperature;
    this->publish_state();
    ESP_LOGD(TAG, "DHW current temperature updated: %.1f°C", temperature);
  }
}

void HeishamonWaterHeater::update_dhw_state(bool heating) {
  if (heating != this->dhw_heating_) {
    this->dhw_heating_ = heating;
    
    // Update action based on heating state
    if (heating) {
      this->action = climate::CLIMATE_ACTION_HEATING;
    } else {
      this->action = climate::CLIMATE_ACTION_IDLE;
    }
    
    this->publish_state();
    ESP_LOGD(TAG, "DHW heating state updated: %s", heating ? "ON" : "OFF");
  }
}

void HeishamonWaterHeater::update_dhw_mode(int mode) {
  if (mode != this->dhw_mode_) {
    this->dhw_mode_ = mode;
    
    // Convert to Climate mode
    switch (mode) {
      case 0:
        this->mode = climate::CLIMATE_MODE_OFF;
        break;
      case 1:
        this->mode = climate::CLIMATE_MODE_HEAT;
        break;
      case 2:
        this->mode = climate::CLIMATE_MODE_AUTO;
        break;
      default:
        ESP_LOGW(TAG, "Unknown DHW mode: %d", mode);
        return;
    }
    
    this->publish_state();
    ESP_LOGD(TAG, "DHW mode updated: %d", mode);
  }
}

void HeishamonWaterHeater::send_target_temperature_() {
  if (this->parent_ != nullptr) {
    // Send command to set DHW target temperature
    std::string command = "SetDHWTemp," + to_string((int)this->target_temperature_);
    this->parent_->send_command(command);
    ESP_LOGD(TAG, "Sent DHW temperature command: %s", command.c_str());
  }
}

void HeishamonWaterHeater::send_dhw_mode_() {
  if (this->parent_ != nullptr) {
    // Send command to set DHW mode
    std::string command = "SetDHWMode," + to_string(this->dhw_mode_);
    this->parent_->send_command(command);
    ESP_LOGD(TAG, "Sent DHW mode command: %s", command.c_str());
  }
}

}  // namespace heishamon
}  // namespace esphome
