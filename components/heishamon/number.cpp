#include "number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.number";

void HeishamonNumber::setup() {
  // Register with parent component
  this->parent_->register_number(this);
  
  // Restore last state if available
  auto restore = this->get_initial_state_with_restore_mode();
  if (restore.has_value()) {
    this->publish_state(*restore);
  }
}

void HeishamonNumber::dump_config() {
  ESP_LOGCONFIG(TAG, "HeishaMon Number '%s':", this->get_name().c_str());
  ESP_LOGCONFIG(TAG, "  Type: %s", this->number_type_.c_str());
  ESP_LOGCONFIG(TAG, "  Command: %s", this->command_.c_str());
  ESP_LOGCONFIG(TAG, "  Range: %.1f - %.1f", this->traits.get_min_value(), this->traits.get_max_value());
  ESP_LOGCONFIG(TAG, "  Step: %.1f", this->traits.get_step());
  if (!this->traits.get_unit_of_measurement().empty()) {
    ESP_LOGCONFIG(TAG, "  Unit: %s", this->traits.get_unit_of_measurement().c_str());
  }
}

void HeishamonNumber::control(float value) {
  // Validate range
  if (value < this->traits.get_min_value() || value > this->traits.get_max_value()) {
    ESP_LOGW(TAG, "Value %.1f is out of range [%.1f - %.1f] for %s", 
             value, this->traits.get_min_value(), this->traits.get_max_value(), this->get_name().c_str());
    return;
  }
  
  // Round to step
  float step = this->traits.get_step();
  if (step > 0) {
    value = round(value / step) * step;
  }
  
  ESP_LOGD(TAG, "Setting %s to %.1f", this->get_name().c_str(), value);
  
  // Send command to HeishaMon
  if (this->parent_->send_number_command(this->command_, value)) {
    this->publish_state(value);
    ESP_LOGD(TAG, "Successfully set %s to %.1f", this->get_name().c_str(), value);
  } else {
    ESP_LOGW(TAG, "Failed to set %s to %.1f", this->get_name().c_str(), value);
  }
}

}  // namespace heishamon
}  // namespace esphome
