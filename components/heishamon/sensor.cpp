#include "sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.sensor";

void HeishamonSensor::on_data_received(float value) {
  if (!isnan(value) && value != -1.0f) {  // -1 is used for "unknown" in HeishaMon
    this->publish_state(value);
  }
}

}  // namespace heishamon
}  // namespace esphome
