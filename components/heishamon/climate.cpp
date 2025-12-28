#include "esphome/core/defines.h"
#ifdef USE_CLIMATE
#include "climate.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.climate";

void HeishaMonClimate::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HeishaMon Climate Zone %d...", this->zone_id_);
  
  // Set default temperature limits
  this->current_temperature = NAN;
  this->target_temperature = 20.0f;  // Default target temperature
  this->mode = climate::CLIMATE_MODE_OFF;
}

void HeishaMonClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "HeishaMon Climate:");
  ESP_LOGCONFIG(TAG, "  Zone ID: %d", this->zone_id_);
  ESP_LOGCONFIG(TAG, "  Supports Heat: %s", YESNO(this->supports_heat_));
  ESP_LOGCONFIG(TAG, "  Supports Cool: %s", YESNO(this->supports_cool_));
}

climate::ClimateTraits HeishaMonClimate::traits() {
  auto traits = climate::ClimateTraits();
  
  // Supported modes
  traits.set_supports_current_temperature(true);
  traits.set_supports_two_point_target_temperature(false);
  
  // Always support OFF mode
  traits.add_supported_mode(climate::CLIMATE_MODE_OFF);
  
  if (this->supports_heat_) {
    traits.add_supported_mode(climate::CLIMATE_MODE_HEAT);
  }
  
  if (this->supports_cool_) {
    traits.add_supported_mode(climate::CLIMATE_MODE_COOL);
  }
  
  // Temperature ranges based on zone type and mode
  if (this->supports_heat_) {
    // Heat mode: water temperature control (typically 20-55°C)
    traits.set_visual_min_temperature(20.0f);
    traits.set_visual_max_temperature(55.0f);
  } else if (this->supports_cool_) {
    // Cool mode: water temperature control (typically 5-25°C)
    traits.set_visual_min_temperature(5.0f);
    traits.set_visual_max_temperature(25.0f);
  }
  
  traits.set_visual_temperature_step(1.0f);
  
  return traits;
}

void HeishaMonClimate::control(const climate::ClimateCall &call) {
  bool changed = false;
  
  // Handle mode change
  if (call.get_mode().has_value()) {
    climate::ClimateMode new_mode = *call.get_mode();
    ESP_LOGD(TAG, "Zone %d mode change: %d -> %d", this->zone_id_, this->mode, new_mode);
    
    if (new_mode != this->mode) {
      this->mode = new_mode;
      changed = true;
      
      // Update heat pump operating mode
      this->update_operating_mode();
    }
  }
  
  // Handle target temperature change
  if (call.get_target_temperature().has_value()) {
    float new_target = *call.get_target_temperature();
    ESP_LOGD(TAG, "Zone %d target temperature: %.1f°C", this->zone_id_, new_target);
    
    if (fabsf(new_target - this->target_temperature) > 0.1f) {
      this->target_temperature = new_target;
      changed = true;
      
      // Send temperature command to heat pump
      this->send_climate_command();
    }
  }
  
  if (changed) {
    this->publish_state();
  }
}

void HeishaMonClimate::update_from_heishamon() {
  if (this->parent_ == nullptr) return;
  
  bool state_changed = false;
  
  // Update current temperature for this zone
  this->update_zone_temperatures();
  
  // Update zone and global operating states
  this->update_zone_states();
  
  // Update HVAC mode based on heat pump state
  climate::ClimateMode new_mode = this->get_current_hvac_mode();
  if (new_mode != this->mode) {
    this->mode = new_mode;
    state_changed = true;
  }
  
  if (state_changed) {
    this->publish_state();
  }
}

void HeishaMonClimate::update_operating_mode() {
  if (this->parent_ == nullptr) return;
  
  ESP_LOGD(TAG, "Updating operating mode for zone %d, mode: %d", this->zone_id_, this->mode);
  
  // Update zone state based on climate mode
  if (this->mode == climate::CLIMATE_MODE_HEAT) {
    // Enable heating for this zone
    if (this->zone_id_ == 1) {
      this->parent_->set_zone1_heat_enabled(true);
    } else if (this->zone_id_ == 2) {
      this->parent_->set_zone2_heat_enabled(true);
    }
    
    // Enable global heating mode
    this->parent_->set_heat_mode_enabled(true);
    
  } else if (this->mode == climate::CLIMATE_MODE_COOL) {
    // Enable cooling for this zone
    if (this->zone_id_ == 1) {
      this->parent_->set_zone1_cool_enabled(true);
    } else if (this->zone_id_ == 2) {
      this->parent_->set_zone2_cool_enabled(true);
    }
    
    // Enable global cooling mode
    this->parent_->set_cool_mode_enabled(true);
    
  } else if (this->mode == climate::CLIMATE_MODE_OFF) {
    // Disable this zone
    if (this->zone_id_ == 1) {
      this->parent_->set_zone1_heat_enabled(false);
      this->parent_->set_zone1_cool_enabled(false);
    } else if (this->zone_id_ == 2) {
      this->parent_->set_zone2_heat_enabled(false);
      this->parent_->set_zone2_cool_enabled(false);
    }
    
    // Check if we should disable global modes
    // (this would need coordination with other zones)
  }
}

void HeishaMonClimate::update_zone_temperatures() {
  if (this->parent_ == nullptr) return;
  
  // Get current temperature for this zone
  float new_current_temp = NAN;
  float new_target_temp = NAN;
  
  if (this->zone_id_ == 1) {
    new_current_temp = this->parent_->get_zone1_current_temperature();
    if (this->supports_heat_) {
      new_target_temp = this->parent_->get_zone1_heat_target_temperature();
    } else if (this->supports_cool_) {
      new_target_temp = this->parent_->get_zone1_cool_target_temperature();
    }
  } else if (this->zone_id_ == 2) {
    new_current_temp = this->parent_->get_zone2_current_temperature();
    if (this->supports_heat_) {
      new_target_temp = this->parent_->get_zone2_heat_target_temperature();
    } else if (this->supports_cool_) {
      new_target_temp = this->parent_->get_zone2_cool_target_temperature();
    }
  }
  
  // Update current temperature
  if (!std::isnan(new_current_temp) && fabsf(new_current_temp - this->current_temperature) > 0.1f) {
    this->current_temperature = new_current_temp;
  }
  
  // Update target temperature if different
  if (!std::isnan(new_target_temp) && fabsf(new_target_temp - this->target_temperature) > 0.1f) {
    this->target_temperature = new_target_temp;
  }
}

void HeishaMonClimate::update_zone_states() {
  if (this->parent_ == nullptr) return;
  
  // Update global operating modes
  this->global_heat_enabled_ = this->parent_->get_heat_mode_enabled();
  this->global_cool_enabled_ = this->parent_->get_cool_mode_enabled();
  
  // Update zone-specific states
  if (this->zone_id_ == 1) {
    this->zone_heat_enabled_ = this->parent_->get_zone1_heat_enabled();
    this->zone_cool_enabled_ = this->parent_->get_zone1_cool_enabled();
  } else if (this->zone_id_ == 2) {
    this->zone_heat_enabled_ = this->parent_->get_zone2_heat_enabled();
    this->zone_cool_enabled_ = this->parent_->get_zone2_cool_enabled();
  }
}

void HeishaMonClimate::send_climate_command() {
  if (this->parent_ == nullptr) return;
  
  ESP_LOGD(TAG, "Sending climate command for zone %d: %.1f°C", this->zone_id_, this->target_temperature);
  
  // Send target temperature command based on zone and mode
  if (this->zone_id_ == 1) {
    if (this->supports_heat_) {
      this->parent_->set_zone1_heat_target_temperature(this->target_temperature);
    } else if (this->supports_cool_) {
      this->parent_->set_zone1_cool_target_temperature(this->target_temperature);
    }
  } else if (this->zone_id_ == 2) {
    if (this->supports_heat_) {
      this->parent_->set_zone2_heat_target_temperature(this->target_temperature);
    } else if (this->supports_cool_) {
      this->parent_->set_zone2_cool_target_temperature(this->target_temperature);
    }
  }
}

bool HeishaMonClimate::is_zone_active() const {
  if (this->supports_heat_ && this->global_heat_enabled_ && this->zone_heat_enabled_) {
    return true;
  }
  if (this->supports_cool_ && this->global_cool_enabled_ && this->zone_cool_enabled_) {
    return true;
  }
  return false;
}

climate::ClimateMode HeishaMonClimate::get_current_hvac_mode() const {
  // Determine current HVAC mode based on heat pump state
  if (this->is_zone_active()) {
    if (this->supports_heat_ && this->global_heat_enabled_ && this->zone_heat_enabled_) {
      return climate::CLIMATE_MODE_HEAT;
    }
    if (this->supports_cool_ && this->global_cool_enabled_ && this->zone_cool_enabled_) {
      return climate::CLIMATE_MODE_COOL;
    }
  }
  
  return climate::CLIMATE_MODE_OFF;
}

}  // namespace heishamon
}  // namespace esphome
#endif
