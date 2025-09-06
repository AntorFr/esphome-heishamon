#ifdef USE_NUMBER
#include "number.h"
#include "heishamon.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.number";

void HeishamonNumber::setup() {
  // Register callback to receive current values if read_topic is set
  if (!this->read_topic_.empty() && this->parent_) {
    ESP_LOGD(TAG, "Registering number %s for topic %s", this->get_name().c_str(), this->read_topic_.c_str());
    this->parent_->register_sensor_callback(this->read_topic_, [this](float value) {
      // Only update the displayed value, don't trigger control
      ESP_LOGD(TAG, "Received value %.1f for number %s", value, this->get_name().c_str());
      this->publish_state(value);
    });
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
#endif
