#include "number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.number";

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
