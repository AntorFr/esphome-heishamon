#pragma once
#include "esphome/core/defines.h"
#ifdef USE_NUMBER

#include "esphome/core/component.h"
#include "esphome/components/number/number.h"

namespace esphome {
namespace heishamon {

// Forward declaration
class HeishamonComponent;

class HeishamonNumber : public number::Number, public Component {
 public:
  void set_parent(HeishamonComponent *parent) { parent_ = parent; }
  void set_number_type(const std::string &type) { number_type_ = type; }
  void set_command(const std::string &command) { command_ = command; }
  void set_read_topic(const std::string &topic) { read_topic_ = topic; }
  
  void set_min_value(float min_value) { this->traits.set_min_value(min_value); }
  void set_max_value(float max_value) { this->traits.set_max_value(max_value); }
  void set_step(float step) { this->traits.set_step(step); }
  void set_unit_of_measurement(const std::string &unit) { 
    this->unit_ = unit;  // Store persistently
    this->traits.set_unit_of_measurement(this->unit_.c_str()); 
  }
  void set_device_class(const std::string &device_class) {
    this->device_class_ = device_class;  // Store persistently  
    this->traits.set_device_class(this->device_class_.c_str()); 
  }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void control(float value) override;
  
  HeishamonComponent *parent_{nullptr};
  std::string number_type_;
  std::string command_;
  std::string read_topic_;  // Topic to read current value from
  std::string unit_;        // Persistent storage for unit
  std::string device_class_; // Persistent storage for device class
};

}  // namespace heishamon
}  // namespace esphome
#endif
