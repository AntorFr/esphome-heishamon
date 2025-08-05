#include "select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.select";

void HeishamonSelect::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heishamon Select '%s'...", this->select_type_.c_str());
  
  // Set initial option if specified
  if (!this->initial_option_.empty()) {
    this->publish_state(this->initial_option_);
  }
}

void HeishamonSelect::dump_config() {
  ESP_LOGCONFIG(TAG, "Heishamon Select:");
  ESP_LOGCONFIG(TAG, "  Type: %s", this->select_type_.c_str());
  ESP_LOGCONFIG(TAG, "  Command: %s", this->command_.c_str());
  LOG_SELECT("  ", "Select", this);
}

void HeishamonSelect::control(const std::string &value) {
  ESP_LOGD(TAG, "Setting %s to %s", this->select_type_.c_str(), value.c_str());
  
  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG, "Parent component not set!");
    return;
  }
  
  // Convert option to command value
  std::string command_value = this->option_to_command_value(value);
  
  if (!command_value.empty()) {
    // Send command via parent component
    this->parent_->send_command(this->command_, command_value);
    this->publish_state(value);
    ESP_LOGD(TAG, "Command sent successfully: %s = %s", this->command_.c_str(), command_value.c_str());
  } else {
    ESP_LOGW(TAG, "Unknown option: %s for select type: %s", value.c_str(), this->select_type_.c_str());
  }
}

std::string HeishamonSelect::option_to_command_value(const std::string &option) {
  if (this->select_type_ == "bivalent_mode") {
    return this->bivalent_mode_to_command(option);
  } else if (this->select_type_ == "external_pad_heater_type") {
    return this->heater_type_to_command(option);
  } else if (this->select_type_ == "smart_grid_mode") {
    return this->smart_grid_to_command(option);
  } else if (this->select_type_ == "heating_mode") {
    return this->heating_mode_to_command(option);
  } else if (this->select_type_ == "cooling_mode") {
    return this->cooling_mode_to_command(option);
  }
  return "";
}

std::string HeishamonSelect::bivalent_mode_to_command(const std::string &option) {
  if (option == "Disabled") return "0";
  if (option == "Alternative") return "1";
  if (option == "Hybrid") return "2";
  return "";
}

std::string HeishamonSelect::heater_type_to_command(const std::string &option) {
  if (option == "Disabled") return "0";
  if (option == "Type 1") return "1";
  if (option == "Type 2") return "2";
  if (option == "Type 3") return "3";
  return "";
}

std::string HeishamonSelect::smart_grid_to_command(const std::string &option) {
  if (option == "Disabled") return "0";
  if (option == "SG Ready") return "1";
  if (option == "Power Control") return "2";
  if (option == "Normal") return "3";
  return "";
}

std::string HeishamonSelect::heating_mode_to_command(const std::string &option) {
  if (option == "Compensated") return "0";
  if (option == "Direct") return "1";
  if (option == "Water") return "2";
  if (option == "Room") return "3";
  return "";
}

std::string HeishamonSelect::cooling_mode_to_command(const std::string &option) {
  if (option == "Compensated") return "0";
  if (option == "Direct") return "1";
  if (option == "Water") return "2";
  if (option == "Room") return "3";
  return "";
}

}  // namespace heishamon
}  // namespace esphome
