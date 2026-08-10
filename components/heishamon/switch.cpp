#include "esphome/core/defines.h"
#ifdef USE_SWITCH
#include "switch.h"
#include "esphome/core/log.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace heishamon {

static const char *const SWITCH_TAG = "heishamon.switch";

void HeishamonSwitch::write_state(bool state) {
  // Topics without an associated SET command are read-only states
  if (this->command_.empty() || this->parent_ == nullptr) {
    ESP_LOGW(SWITCH_TAG, "Switch is read-only - state change ignored");
    this->publish_state(this->state);
    return;
  }

  ESP_LOGD(SWITCH_TAG, "Sending command %s = %d", this->command_.c_str(), state ? 1 : 0);
  if (this->parent_->send_command(this->command_, static_cast<uint8_t>(state ? 1 : 0))) {
    // Optimistic update; the next pump poll confirms the real state
    this->publish_state(state);
  } else {
    // Command rejected (e.g. listen-only mode): keep the current state
    this->publish_state(this->state);
  }
}

}  // namespace heishamon
}  // namespace esphome
#endif
