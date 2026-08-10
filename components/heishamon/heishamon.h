#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include "heishamon_protocol.h"
#include "heishamon_callbacks.h"
#include <vector>
#include <map>
#include <string>
#include <functional>

// Conditional includes for optional components
#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif
#ifdef USE_WATER_HEATER
#include "esphome/components/water_heater/water_heater.h"
#endif

namespace esphome {
namespace heishamon {

// Forward declarations
#ifdef USE_CLIMATE
class HeishaMonClimate;
#endif
#ifdef USE_NUMBER
class HeishamonNumber;
#endif
#ifdef USE_WATER_HEATER
class HeishamonWaterHeater;
#endif

// Structure for topics and their data
struct HeishaTopic {
  std::string name;
  uint8_t byte_index;
  std::function<float(uint8_t)> decode_func;
  std::string unit;
  std::string description;
};

// Structure for topics and their data
struct TopicData {
  std::string topic;
  float value;
  bool valid;
};

// Zone Sensor Mode enum (from TOP111/TOP112)
enum class ZoneSensorMode : uint8_t {
  WATER = 0,
  EXT_THERMO = 1,
  INT_THERMO = 2,
  THERMISTOR = 3
};

class HeishamonComponent : public Component, public uart::UARTDevice {
 public:
  HeishamonComponent();
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Configuration
  void set_update_interval(uint32_t interval) { this->update_interval_ = interval; }
  void set_listen_only(bool listen_only) { 
    this->listen_only_ = listen_only; 
    if (this->protocol_) {
      this->protocol_->set_listen_only(listen_only);
    }
  }
  void set_optional_pcb(bool optional_pcb) {
    this->optional_pcb_ = optional_pcb;
    if (this->protocol_) {
      this->protocol_->set_optional_pcb(optional_pcb);
    }
  }
  void set_tx_enable_pin(GPIOPin *pin) { this->tx_enable_pin_ = pin; }

  // Public functions for sensors
  void register_sensor_callback(const std::string &topic, std::function<void(float)> &&callback);
  void register_binary_sensor_callback(const std::string &topic, std::function<void(bool)> callback);
  void register_switch_callback(const std::string &topic, std::function<void(bool)> callback);
  void register_select_callback(const std::string &topic, std::function<void(const std::string&)> callback);
  void register_text_sensor_callback(const std::string &topic, std::function<void(const std::string&)> callback);

  // Command sending functions
  void send_command(const std::string &command);
  void send_command(const std::string &command, const std::string &value);
  bool send_command(const std::string &command, uint8_t value);
  bool send_command(const std::vector<uint8_t> &command);
  void create_command(const std::string &command, uint8_t value);
  bool send_number_command(const std::string &command, float value);

  // Climate control functions
  void set_heat_mode_enabled(bool enabled);
  void set_cool_mode_enabled(bool enabled);
  void set_zone1_heat_enabled(bool enabled);
  void set_zone1_cool_enabled(bool enabled);
  void set_zone2_heat_enabled(bool enabled);
  void set_zone2_cool_enabled(bool enabled);
  
  // Temperature setters
  void set_zone1_heat_target_temperature(float temperature);
  void set_zone1_cool_target_temperature(float temperature);
  void set_zone2_heat_target_temperature(float temperature);
  void set_zone2_cool_target_temperature(float temperature);
  
  // Temperature getters
  float get_zone1_current_temperature() const;
  float get_zone2_current_temperature() const;
  float get_zone1_heat_target_temperature() const;
  float get_zone1_cool_target_temperature() const;
  float get_zone2_heat_target_temperature() const;
  float get_zone2_cool_target_temperature() const;
  
  // State getters
  bool get_heat_mode_enabled() const;
  bool get_cool_mode_enabled() const;
  bool get_zone1_heat_enabled() const;
  bool get_zone1_cool_enabled() const;
  bool get_zone2_heat_enabled() const;
  bool get_zone2_cool_enabled() const;
  
  // Zone sensor mode getters
  ZoneSensorMode get_zone1_sensor_mode() const { return this->zone1_sensor_mode_; }
  ZoneSensorMode get_zone2_sensor_mode() const { return this->zone2_sensor_mode_; }
  
  // DHW (Domestic Hot Water) getters
  float get_dhw_current_temperature() const { return this->dhw_current_temp_; }
  float get_dhw_target_temperature() const { return this->dhw_target_temp_; }
  bool get_dhw_heating_state() const { return this->dhw_heating_state_; }
  int get_dhw_mode() const { return this->dhw_mode_; }
  
  // Optional component registration - only defined if components are used
#ifdef USE_CLIMATE
  void register_climate_component(class HeishaMonClimate *climate);
#endif
#ifdef USE_WATER_HEATER
  void register_water_heater(class HeishamonWaterHeater *water_heater);
#endif
  
#ifdef USE_NUMBER
  void register_number(class HeishamonNumber *number);
#endif

 protected:
  // Protocol layer data callback
  void on_protocol_data_received(const std::vector<uint8_t> &data, uint8_t data_type);
  
  // Data processing
  void decode_and_notify_sensors(const std::vector<uint8_t> &data);
  
  // Core components
  HeishamonProtocol *protocol_{nullptr};
  HeishamonCallbackManager *callback_manager_{nullptr};
  
  // Configuration
  uint32_t update_interval_{15000};
  bool listen_only_{false};
  bool optional_pcb_{false};
  GPIOPin *tx_enable_pin_{nullptr};
  
  // Timing
  uint32_t last_run_time_{0};
  uint32_t last_optional_pcb_time_{0};
  bool initial_query_sent_{false};
  
  // Optional components - only defined if the component is used
#ifdef USE_CLIMATE
  std::vector<class HeishaMonClimate *> climate_components_;
#endif
  
#ifdef USE_NUMBER
  std::vector<class HeishamonNumber *> number_components_;
#endif
  
#ifdef USE_WATER_HEATER
  std::vector<class HeishamonWaterHeater *> water_heater_components_;
#endif
  
  // Climate state tracking
  bool heat_mode_enabled_{false};
  bool cool_mode_enabled_{false};
  bool zone1_heat_enabled_{false};
  bool zone1_cool_enabled_{false};
  bool zone2_heat_enabled_{false};
  bool zone2_cool_enabled_{false};
  
  // Current temperatures
  float zone1_current_temp_{NAN};
  float zone2_current_temp_{NAN};
  float zone1_heat_target_temp_{20.0f};
  float zone1_cool_target_temp_{20.0f};
  float zone2_heat_target_temp_{20.0f};
  float zone2_cool_target_temp_{20.0f};
  
  // DHW state tracking
  float dhw_current_temp_{NAN};
  float dhw_target_temp_{45.0f};
  bool dhw_heating_state_{false};
  int dhw_mode_{0};

  // Zone sensor modes (TOP111/TOP112)
  ZoneSensorMode zone1_sensor_mode_{ZoneSensorMode::WATER};
  ZoneSensorMode zone2_sensor_mode_{ZoneSensorMode::WATER};

  // Decoding and data processing
  void decode_heatpump_data(const std::vector<uint8_t> &data);
  void decode_optional_data(const std::vector<uint8_t> &data);
  
  // Decoding functions (ported from HeishaMon)
  float unknown(uint8_t input);
  float get_bit_1_and_2(uint8_t input);
  float get_bit_3_and_4(uint8_t input);
  float get_bit_5_and_6(uint8_t input);
  float get_bit_7_and_8(uint8_t input);
  float get_int_minus_1(uint8_t input);
  float get_int_minus_128(uint8_t input);
  float get_int_minus_1_div_5(uint8_t input);
  float get_int_minus_1_div_50(uint8_t input);
  float get_int_minus_1_times_10(uint8_t input);
  float get_int_minus_1_times_50(uint8_t input);
  float get_power(uint8_t input);
  float get_pump_flow(const std::vector<uint8_t> &data);
  
  // Additional decoding functions for Phase 1 sensors
  float get_dhw_power(uint8_t input);
  float get_heat_delta(uint8_t input);
  float get_cool_delta(uint8_t input);
  float get_operating_hours(uint8_t input);
  float get_cop(const std::vector<uint8_t> &data);
  float get_zone_valve_pid(uint8_t input);
  
  // Select value decoding functions
  std::string decode_operation_mode(uint8_t input);
  
  // Topics définis
  std::vector<HeishaTopic> topics_;
  std::vector<HeishaTopic> optional_topics_;
};

}  // namespace heishamon
}  // namespace esphome
