#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "heishamon.h"

namespace esphome {
namespace heishamon {

class HeishamonSensor : public sensor::Sensor {
 public:
  void set_parent(HeishamonComponent *parent) { this->parent_ = parent; }
  void set_topic(const std::string &topic) { this->topic_ = topic; }

 protected:
  HeishamonComponent *parent_;
  std::string topic_;
  
  void on_data_received(float value);
};

}  // namespace heishamon
}  // namespace esphome
