#include "binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.binary_sensor";

void HeishamonBinarySensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heishamon Binary Sensor '%s'...", this->topic_.c_str());
  
  // Register callback with parent
  if (this->parent_ != nullptr) {
    this->parent_->register_sensor(this->topic_, [this](float value) {
      this->on_data_received(value);
    });
  } else {
    ESP_LOGE(TAG, "Parent component not set!");
  }
}

void HeishamonBinarySensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Heishamon Binary Sensor:");
  ESP_LOGCONFIG(TAG, "  Topic: %s", this->topic_.c_str());
  LOG_BINARY_SENSOR("  ", "Binary Sensor", this);
}

void HeishamonBinarySensor::on_data_received(float value) {
  if (value != -1.0f) {  // -1 is used for "unknown" in HeishaMon
    this->publish_state(value > 0);
  }
}

}  // namespace heishamon
}  // namespace esphome
