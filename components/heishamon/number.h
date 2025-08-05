#pragma once

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"
#include "heishamon.h"

namespace esphome {
namespace heishamon {

class HeishamonNumber : public number::Number, public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  
  void set_parent(HeishamonComponent *parent) { this->parent_ = parent; }
  void set_number_type(const std::string &type) { this->number_type_ = type; }
  void set_command(const std::string &command) { this->command_ = command; }
  void set_min_value(float min_value) { this->traits.set_min_value(min_value); }
  void set_max_value(float max_value) { this->traits.set_max_value(max_value); }
  void set_step(float step) { this->traits.set_step(step); }
  void set_unit_of_measurement(const std::string &unit) { this->traits.set_unit_of_measurement(unit); }
  void set_device_class(const std::string &device_class) { this->traits.set_device_class(device_class); }

 protected:
  void control(float value) override;
  
  HeishamonComponent *parent_;
  std::string number_type_;
  std::string command_;
};

}  // namespace heishamon
}  // namespace esphome
