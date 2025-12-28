#include "esphome/core/defines.h"
// Using USE_CLIMATE until ESPHome supports water_heater natively
#ifdef USE_CLIMATE
#include "water_heater.h"
#include "heishamon.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.water_heater";

// Custom preset names for DHW modes
const char* HeishamonWaterHeater::PRESET_NORMAL = "Normal";
const char* HeishamonWaterHeater::PRESET_ECO = "Eco";
const char* HeishamonWaterHeater::PRESET_POWERFUL = "Powerful";

void HeishamonWaterHeater::setup() {
  // Set initial state - DHW is typically always in HEAT mode
  this->target_temperature = this->target_temperature_;
  this->current_temperature = NAN;
  this->mode = climate::CLIMATE_MODE_HEAT;  // DHW is always in heat mode
  this->action = climate::CLIMATE_ACTION_IDLE;
  this->set_custom_preset_(PRESET_NORMAL);  // Default to Normal preset
  
  ESP_LOGCONFIG(TAG, "HeishaMon DHW (Water Heater) setup complete");
}

void HeishamonWaterHeater::dump_config() {
  ESP_LOGCONFIG(TAG, "HeishaMon DHW (Water Heater):");
  ESP_LOGCONFIG(TAG, "  Target Temperature: %.1f°C", this->target_temperature_);
  ESP_LOGCONFIG(TAG, "  Presets: Normal, Eco, Powerful");
}

climate::ClimateTraits HeishamonWaterHeater::traits() {
  auto traits = climate::ClimateTraits();
  
  // Supported features
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_ACTION);
  
  // Supported modes - DHW is essentially always heating
  traits.add_supported_mode(climate::CLIMATE_MODE_HEAT);  // DHW Active
  traits.add_supported_mode(climate::CLIMATE_MODE_OFF);   // DHW Disabled
  
  // Custom presets for DHW operating modes (Performance/Normal/Eco)
  traits.set_supported_custom_presets({PRESET_NORMAL, PRESET_ECO, PRESET_POWERFUL});
  
  // Temperature settings for DHW (40-75°C per HeishaMon protocol)
  traits.set_visual_min_temperature(40.0f);
  traits.set_visual_max_temperature(75.0f);
  traits.set_visual_temperature_step(1.0f);
  
  return traits;
}

void HeishamonWaterHeater::control(const climate::ClimateCall &call) {
  bool changed = false;
  
  // Handle mode changes (ON/OFF)
  if (call.get_mode().has_value()) {
    climate::ClimateMode new_mode = *call.get_mode();
    if (new_mode != this->mode) {
      this->mode = new_mode;
      changed = true;
      
      // When turned OFF, stop force DHW
      if (new_mode == climate::CLIMATE_MODE_OFF) {
        this->force_dhw_ = false;
        if (this->parent_ != nullptr) {
          this->parent_->send_command("SetForceDHW", (uint8_t)0);
          ESP_LOGD(TAG, "DHW disabled - sent SetForceDHW=0");
        }
      }
    }
  }
  
  // Handle custom preset changes (Normal/Eco/Powerful)
  if (call.get_custom_preset() != nullptr) {
    std::string new_preset = call.get_custom_preset();
    const char* current_preset = this->get_custom_preset();
    if (current_preset == nullptr || std::string(current_preset) != new_preset) {
      this->set_custom_preset_(new_preset.c_str());
      this->dhw_operating_mode_ = preset_string_to_mode_(new_preset);
      changed = true;
      
      this->send_dhw_operating_mode_();
      ESP_LOGD(TAG, "DHW operating mode changed to: %s (%d)", 
               new_preset.c_str(), static_cast<int>(this->dhw_operating_mode_));
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
  int new_mode = this->parent_->get_dhw_mode();
  
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
    } else {
      this->action = climate::CLIMATE_ACTION_IDLE;
    }
    changed = true;
  }
  
  // Update operating mode preset from heat pump
  // DHW mode from protocol: typically 0=Normal, 1=Eco, 2=Powerful
  DHWOperatingMode new_operating_mode = static_cast<DHWOperatingMode>(new_mode);
  if (new_operating_mode != this->dhw_operating_mode_) {
    this->dhw_operating_mode_ = new_operating_mode;
    std::string preset_str = mode_to_preset_string_(new_operating_mode);
    this->set_custom_preset_(preset_str.c_str());
    changed = true;
    ESP_LOGD(TAG, "DHW operating mode updated from heat pump: %s", preset_str.c_str());
  }
  
  if (changed) {
    const char* preset = this->get_custom_preset();
    ESP_LOGD(TAG, "DHW state update: action=%d, current=%.1f, target=%.1f, preset=%s",
             this->action, this->current_temperature, this->target_temperature,
             preset != nullptr ? preset : "none");
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

void HeishamonWaterHeater::send_dhw_operating_mode_() {
  if (this->parent_ != nullptr) {
    // Send command to set DHW operating mode
    // 0=Normal, 1=Eco, 2=Powerful
    uint8_t mode_value = static_cast<uint8_t>(this->dhw_operating_mode_);
    this->parent_->send_command("SetDHWMode", mode_value);
    ESP_LOGD(TAG, "Sent DHW operating mode command: %d", mode_value);
  }
}

std::string HeishamonWaterHeater::mode_to_preset_string_(DHWOperatingMode mode) {
  switch (mode) {
    case DHWOperatingMode::ECO:
      return PRESET_ECO;
    case DHWOperatingMode::POWERFUL:
      return PRESET_POWERFUL;
    case DHWOperatingMode::NORMAL:
    default:
      return PRESET_NORMAL;
  }
}

DHWOperatingMode HeishamonWaterHeater::preset_string_to_mode_(const std::string &preset) {
  if (preset == PRESET_ECO) {
    return DHWOperatingMode::ECO;
  } else if (preset == PRESET_POWERFUL) {
    return DHWOperatingMode::POWERFUL;
  }
  return DHWOperatingMode::NORMAL;
}

}  // namespace heishamon
}  // namespace esphome
#endif
