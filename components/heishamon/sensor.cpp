#include "sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.sensor";

void HeishamonSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heishamon Sensor '%s'...", this->topic_.c_str());
  
  // Enregistrer le callback avec le parent
  if (this->parent_ != nullptr) {
    this->parent_->register_sensor(this->topic_, [this](float value) {
      this->on_data_received(value);
    });
  } else {
    ESP_LOGE(TAG, "Parent component not set!");
  }
}

void HeishamonSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Heishamon Sensor:");
  ESP_LOGCONFIG(TAG, "  Topic: %s", this->topic_.c_str());
  LOG_SENSOR("  ", "Sensor", this);
}

void HeishamonSensor::on_data_received(float value) {
  if (!isnan(value) && value != -1.0f) {  // -1 est utilisé pour "unknown" dans HeishaMon
    this->publish_state(value);
  }
}

}  // namespace heishamon
}  // namespace esphome
