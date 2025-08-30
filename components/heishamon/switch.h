#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "heishamon.h"

namespace esphome {
namespace heishamon {

class HeishamonSwitch : public Component, public switch_::Switch {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_parent(HeishamonComponent *parent) { this->parent_ = parent; }
  void set_command(const std::string &command) { this->command_ = command; }

 protected:
  void write_state(bool state) override;
  
  HeishamonComponent *parent_;
  std::string command_;
  
  std::vector<uint8_t> create_command(bool state);
};

}  // namespace heishamon
}  // namespace esphome
