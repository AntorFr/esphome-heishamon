#pragma once
#include "esphome/core/defines.h"
#ifdef USE_SWITCH

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/log.h"
#include "heishamon.h"

namespace esphome {
namespace heishamon {

class HeishamonSwitch : public switch_::Switch {
 public:
  HeishamonSwitch() : switch_::Switch() {}

  void set_parent(HeishamonComponent *parent) { this->parent_ = parent; }
  void set_command(const std::string &command) { this->command_ = command; }

  // Implementation in switch.cpp
  void write_state(bool state) override;

 protected:
  HeishamonComponent *parent_{nullptr};
  std::string command_;
};

}  // namespace heishamon
}  // namespace esphome
#endif
