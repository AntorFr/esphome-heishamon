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

  void setup() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void control(float value) override;
  
  HeishamonComponent *parent_{nullptr};
  std::string number_type_;
  std::string command_;
  std::string read_topic_;  // Topic to read current value from
};

}  // namespace heishamon
}  // namespace esphome
#endif
