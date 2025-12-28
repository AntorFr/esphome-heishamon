#include "esphome/core/defines.h"
#ifdef USE_SWITCH
#include "switch.h"
#include "esphome/core/log.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace heishamon {

static const char *const SWITCH_TAG = "heishamon.switch";

void HeishamonSwitch::write_state(bool state) {
  // Read-only switch - cannot write state in listen_only mode
  // This method is required by switch_::Switch interface
  ESP_LOGW(SWITCH_TAG, "Switch is read-only - state change ignored");
  // Just publish the current state back (no actual change)
  this->publish_state(this->state);
}

}  // namespace heishamon
}  // namespace esphome
#endif
