#include "esphome/core/defines.h"
#ifdef USE_SELECT
#include "select.h"
#include "esphome/core/log.h"
#include <map>

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.select";

// Maps for converting option strings to numeric command values
static const std::map<std::string, std::map<std::string, uint8_t>> SELECT_VALUE_MAPS = {
    // SET3 - Quiet Mode
    {"quiet_mode", {
        {"Off", 0}, {"Level 1", 1}, {"Level 2", 2}, {"Level 3", 3}, {"Scheduled", 4}
    }},
    // SET4 - Powerful Mode (0/30/60/90 min)
    {"powerful_mode", {
        {"Off", 0}, {"30 min", 1}, {"60 min", 2}, {"90 min", 3}
    }},
    // SET9 - Operating Mode
    {"operating_mode", {
        {"Heat only", 0}, {"Cool only", 1}, {"Auto(Heat)", 2}, {"DHW only", 3},
        {"Heat+DHW", 4}, {"Cool+DHW", 5}, {"Auto(Heat)+DHW", 6},
        {"Auto(Cool)", 7}, {"Auto(Cool)+DHW", 8}
    }},
    // SET17 - Active Zones
    {"zones", {
        {"Zone 1 only", 0}, {"Zone 2 only", 1}, {"Zone 1 + Zone 2", 2}
    }},
    // SET35 - Bivalent Mode
    {"bivalent_mode", {
        {"Alternative", 0}, {"Parallel", 1}, {"Advanced Parallel", 2}
    }},
    // SET26 - External Pad Heater
    {"external_pad_heater", {
        {"Disabled", 0}, {"Type-A", 1}, {"Type-B", 2}
    }},
    // Smart Grid Mode
    {"smart_grid_mode", {
        {"Normal", 0}, {"Capacity 1", 1}, {"HP/DHW Off", 2}, {"Capacity 2", 3}
    }},
    // Read-only selects (for decoding received values)
    {"heating_mode", {
        {"Compensation curve", 0}, {"Direct", 1}
    }},
    {"cooling_mode", {
        {"Compensation curve", 0}, {"Direct", 1}
    }},
    {"three_way_valve", {
        {"Room", 0}, {"DHW", 1}
    }},
    {"holiday_mode_state", {
        {"Off", 0}, {"Scheduled", 1}, {"Active", 2}
    }},
    {"solar_mode", {
        {"Disabled", 0}, {"Buffer", 1}, {"DHW", 2}
    }},
    {"pump_flowrate_mode", {
        {"DeltaT", 0}, {"Max flow", 1}
    }},
    {"liquid_type", {
        {"Water", 0}, {"Glycol", 1}
    }},
    {"z1_sensor_settings", {
        {"Water temperature", 0}, {"External thermostat", 1}, {"Internal thermostat", 2}, {"Thermistor", 3}
    }},
    {"z2_sensor_settings", {
        {"Water temperature", 0}, {"External thermostat", 1}, {"Internal thermostat", 2}, {"Thermistor", 3}
    }},
    // SET43 - DHW Sensor Selection (K/L series All-In-One)
    {"dhw_sensor_selection", {
        {"Top", 0}, {"Center", 1}
    }},
    // SET44 - DHW Heater State
    {"dhw_heater_state", {
        {"Blocked", 0}, {"Free", 1}
    }},
    // SET45 - Room Heater State
    {"room_heater_state", {
        {"Blocked", 0}, {"Free", 1}
    }},
};

// Reverse map for converting numeric values to option strings
static std::string value_to_option(const std::string &select_type, uint8_t value) {
    auto type_it = SELECT_VALUE_MAPS.find(select_type);
    if (type_it == SELECT_VALUE_MAPS.end()) {
        return "";
    }
    for (const auto &pair : type_it->second) {
        if (pair.second == value) {
            return pair.first;
        }
    }
    return "";
}

void HeishamonSelect::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heishamon Select '%s'...", this->select_type_.c_str());
  
  // Register callback for topic-based updates (works in both modes)
  if (!this->topic_.empty() && this->parent_ != nullptr) {
    ESP_LOGCONFIG(TAG, "Registering callback for topic: %s", this->topic_.c_str());
    this->parent_->register_select_callback(this->topic_, [this](const std::string &value) {
      ESP_LOGD(TAG, "Received value for %s: %s", this->select_type_.c_str(), value.c_str());
      // Try to convert numeric value to option string
      char *end;
      long num_value = strtol(value.c_str(), &end, 10);
      if (*end == '\0') {
        // It's a numeric value, convert to option string
        std::string option = value_to_option(this->select_type_, static_cast<uint8_t>(num_value));
        if (!option.empty()) {
          this->publish_state(option);
          return;
        }
      }
      // Otherwise use value as-is (already an option string)
      this->publish_state(value);
    });
  }
  
  // Set initial option if specified
  if (!this->initial_option_.empty()) {
    this->publish_state(this->initial_option_);
  }
}

void HeishamonSelect::dump_config() {
  ESP_LOGCONFIG(TAG, "Heishamon Select:");
  ESP_LOGCONFIG(TAG, "  Type: %s", this->select_type_.c_str());
  if (this->listen_only_ && !this->topic_.empty()) {
    ESP_LOGCONFIG(TAG, "  Mode: Listen-only");
    ESP_LOGCONFIG(TAG, "  Topic: %s", this->topic_.c_str());
  } else {
    ESP_LOGCONFIG(TAG, "  Mode: Command-based");
    ESP_LOGCONFIG(TAG, "  Command: %s", this->command_.c_str());
  }
}

void HeishamonSelect::control(const std::string &value) {
  ESP_LOGD(TAG, "HeishamonSelect control called with value: %s for type: %s", 
           value.c_str(), this->select_type_.c_str());
  
  // In listen-only mode, just log and don't send command
  if (this->listen_only_) {
    ESP_LOGW(TAG, "Select '%s' is read-only in listen-only mode", this->select_type_.c_str());
    return;
  }
  
  // Convert option string to numeric value
  auto type_it = SELECT_VALUE_MAPS.find(this->select_type_);
  if (type_it == SELECT_VALUE_MAPS.end()) {
    ESP_LOGE(TAG, "Unknown select type: %s", this->select_type_.c_str());
    return;
  }
  
  auto value_it = type_it->second.find(value);
  if (value_it == type_it->second.end()) {
    ESP_LOGE(TAG, "Unknown option '%s' for select type '%s'", value.c_str(), this->select_type_.c_str());
    return;
  }
  
  uint8_t cmd_value = value_it->second;
  ESP_LOGD(TAG, "Sending command %s with value %d", this->command_.c_str(), cmd_value);
  
  // Send command via parent component
  if (this->parent_ != nullptr) {
    this->parent_->send_command(this->command_, cmd_value);
    // Optimistically update state
    this->publish_state(value);
  } else {
    ESP_LOGE(TAG, "Parent component not set, cannot send command");
  }
}

}  // namespace heishamon
}  // namespace esphome
#endif
