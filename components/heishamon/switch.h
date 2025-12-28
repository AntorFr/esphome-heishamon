#pragma once
#include "esphome/core/defines.h"
#ifdef USE_SWITCH

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

class HeishamonSwitch : public switch_::Switch {
 public:
  HeishamonSwitch() : switch_::Switch() {}
  
  // Implementation in switch.cpp
  void write_state(bool state) override;
};

}  // namespace heishamon
}  // namespace esphome
#endif
