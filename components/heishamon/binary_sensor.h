#pragma once
#include "esphome/core/defines.h"
#ifdef USE_BINARY_SENSOR

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace heishamon {

class HeishamonBinarySensor : public binary_sensor::BinarySensor {
  // Simple binary sensor - no additional methods needed
};

}  // namespace heishamon
}  // namespace esphome
#endif
