#include "switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.switch";

void HeishamonSwitch::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heishamon Switch '%s'...", this->command_.c_str());
}

void HeishamonSwitch::dump_config() {
  ESP_LOGCONFIG(TAG, "Heishamon Switch:");
  ESP_LOGCONFIG(TAG, "  Command: %s", this->command_.c_str());
  LOG_SWITCH("  ", "Switch", this);
}

void HeishamonSwitch::write_state(bool state) {
  ESP_LOGD(TAG, "Setting %s to %s", this->command_.c_str(), state ? "ON" : "OFF");
  
  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG, "Parent component not set!");
    return;
  }
  
  // Créer la commande appropriée
  std::vector<uint8_t> command = this->create_command(state);
  
  if (!command.empty()) {
    if (this->parent_->send_command(command)) {
      this->publish_state(state);
      ESP_LOGD(TAG, "Command sent successfully");
    } else {
      ESP_LOGW(TAG, "Failed to send command");
    }
  } else {
    ESP_LOGW(TAG, "Unknown command: %s", this->command_.c_str());
  }
}

std::vector<uint8_t> HeishamonSwitch::create_command(bool state) {
  // Commande Panasonic de base (issue de commands.cpp)
  std::vector<uint8_t> cmd = {
    0xf1, 0x6c, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
  };
  
  // Configurer selon la commande (portées depuis commands.cpp)
  if (this->command_ == "force_dhw") {
    cmd[4] = state ? 0x80 : 0x40;  // Force DHW mode
  } 
  else if (this->command_ == "force_defrost") {
    cmd[8] = state ? 0x02 : 0x00;  // Force defrost mode
  }
  else if (this->command_ == "holiday_mode") {
    cmd[5] = state ? 0x20 : 0x10;  // Holiday mode
  }
  else if (this->command_ == "heatpump_state") {
    cmd[4] = state ? 0x02 : 0x01;  // Heat pump state
  }
  else {
    // Commande inconnue
    return {};
  }
  
  return cmd;
}

}  // namespace heishamon
}  // namespace esphome
