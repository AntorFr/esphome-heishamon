#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "heishamon.h"

namespace esphome {
namespace heishamon {

class HeishamonBinarySensor : public binary_sensor::BinarySensor {
 public:
  void set_parent(HeishamonComponent *parent) { this->parent_ = parent; }
  void set_topic(const std::string &topic) { this->topic_ = topic; }

 protected:
  HeishamonComponent *parent_;
  std::string topic_;
};

}  // namespace heishamon
}  // namespace esphome
