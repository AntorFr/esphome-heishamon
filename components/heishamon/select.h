#pragma once

#include "esphome/core/component.h"
#include "esphome/components/select/select.h"
#include "heishamon.h"

namespace esphome {
namespace heishamon {

class HeishamonSelect : public select::Select, public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_parent(HeishamonComponent *parent) { this->parent_ = parent; }
  void set_select_type(const std::string &select_type) { this->select_type_ = select_type; }
  void set_command(const std::string &command) { this->command_ = command; }
  void set_options(const std::vector<std::string> &options) { this->traits.set_options(options); }
  void set_initial_option(const std::string &option) { this->initial_option_ = option; }

 protected:
  void control(const std::string &value) override;
  
  HeishamonComponent *parent_;
  std::string select_type_;
  std::string command_;
  std::string initial_option_;
  
  // Conversion functions for different select types
  std::string bivalent_mode_to_command(const std::string &option);
  std::string heater_type_to_command(const std::string &option);
  std::string smart_grid_to_command(const std::string &option);
  std::string heating_mode_to_command(const std::string &option);
  std::string cooling_mode_to_command(const std::string &option);
  
  // Command value conversion
  std::string option_to_command_value(const std::string &option);
};

}  // namespace heishamon
}  // namespace esphome
