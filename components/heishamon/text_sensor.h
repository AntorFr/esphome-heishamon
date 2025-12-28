#pragma once

#include "esphome/core/defines.h"
#ifdef USE_TEXT_SENSOR

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace heishamon {

class HeishamonTextSensor : public text_sensor::TextSensor {
  // Simple text sensor - no additional methods needed
};

}  // namespace heishamon
}  // namespace esphome

#endif  // USE_TEXT_SENSOR
