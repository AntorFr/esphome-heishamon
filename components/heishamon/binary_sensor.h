#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "heishamon.h"

namespace esphome {
namespace heishamon {

class HeishamonBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_parent(HeishamonComponent *parent) { this->parent_ = parent; }
  void set_topic(const std::string &topic) { this->topic_ = topic; }

 protected:
  HeishamonComponent *parent_;
  std::string topic_;
  
  void on_data_received(float value);
};

}  // namespace heishamon
}  // namespace esphome
