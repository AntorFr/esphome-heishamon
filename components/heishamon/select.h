#pragma once
#include "esphome/core/defines.h"
#ifdef USE_SELECT

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
  void set_topic(const std::string &topic) { this->topic_ = topic; }
  void set_initial_option(const std::string &option) { this->initial_option_ = option; }
  void set_listen_only(bool listen_only) { this->listen_only_ = listen_only; }
  
 protected:
  void control(const std::string &value) override;

  HeishamonComponent *parent_{nullptr};
  std::string select_type_;
  std::string command_;
  std::string topic_;
  std::string initial_option_;
  bool listen_only_{false};
};

}  // namespace heishamon
}  // namespace esphome

#endif
