#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/climate/climate.h"
#include "esphome/core/log.h"
#include <vector>
#include <map>
#include <string>

namespace esphome {
namespace heishamon {

// Forward declarations
class HeishaMonClimate;
class HeishamonNumber;
class HeishamonWaterHeater;
class HeishamonWaterHeater;

// Constants from HeishaMon
#define DATASIZE 203
#define OPTDATASIZE 20
#define MAXDATASIZE 255
#define INITIALQUERYSIZE 7
#define PANASONICQUERYSIZE 110
#define OPTIONALPCBQUERYSIZE 19
#define SERIALTIMEOUT 2000
#define MAXCOMMANDSINBUFFER 10

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

// Structure for buffered commands
struct CommandBuffer {
  uint8_t data[MAXDATASIZE];
  size_t size;
  uint32_t timestamp;
  int retry_count;
};

class HeishamonComponent : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

    // Configuration
  void set_update_interval(uint32_t interval) { this->update_interval_ = interval; }
  void set_listen_only(bool listen_only) { this->listen_only_ = listen_only; }
  void set_optional_pcb(bool optional_pcb) { this->optional_pcb_ = optional_pcb; }

  // Public functions for sensors
  void register_sensor_callback(const std::string &topic, std::function<void(float)> callback);
  void register_binary_sensor_callback(const std::string &topic, std::function<void(bool)> callback);

  // Command sending functions
  void send_command(const std::string &command);
  void send_command(const std::string &command, const std::string &value);
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
  
  // Climate component registration
  void register_climate_component(class HeishaMonClimate *climate);
  
  // Number component registration and support
  void register_number(class HeishamonNumber *number);
  
  // Water Heater component registration and support
  void register_water_heater(HeishamonWaterHeater *water_heater);
  float get_dhw_current_temperature() const;
  float get_dhw_target_temperature() const;
  bool get_dhw_heating_state() const;
  int get_dhw_mode() const;

 protected:
  uint32_t update_interval_{30000};
  bool listen_only_{false};
  bool optional_pcb_{false};
  
  // État de communication
  bool sending_{false};
  bool extra_data_available_{false};
  uint32_t last_run_time_{0};
  uint32_t last_optional_pcb_time_{0};
  uint32_t send_command_read_time_{0};
  
  // Data buffers
  std::vector<uint8_t> data_buffer_;
  std::vector<uint8_t> act_data_;
  std::vector<uint8_t> act_data_extra_;
  std::vector<uint8_t> act_opt_data_;
  
  // Command buffer
  std::vector<CommandBuffer> command_buffer_;
  size_t cmd_start_{0};
  size_t cmd_end_{0};
  size_t cmd_count_{0};
  
  // Callbacks des sensors
  std::map<std::string, std::function<void(float)>> sensor_callbacks_;
  std::map<std::string, std::function<void(bool)>> binary_sensor_callbacks_;
  
  // Climate components
  std::vector<class HeishaMonClimate *> climate_components_;
  
  // Number components
  std::vector<class HeishamonNumber *> number_components_;
  
  // Water Heater components
  std::vector<HeishamonWaterHeater *> water_heater_components_;
  
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
  
  // Statistiques
  uint32_t total_reads_{0};
  uint32_t good_reads_{0};
  uint32_t bad_crc_reads_{0};
  uint32_t bad_header_reads_{0};
  uint32_t timeout_reads_{0};

  // Fonctions privées
  void init_topics();
  bool read_serial();
  void send_panasonic_query();
  void send_optional_pcb_query();
  void decode_heatpump_data(const std::vector<uint8_t> &data);
  void decode_optional_data(const std::vector<uint8_t> &data);
  uint8_t calc_checksum(const std::vector<uint8_t> &command);
  bool is_valid_checksum(const std::vector<uint8_t> &data);
  void push_command_buffer(const std::vector<uint8_t> &command);
  void pop_command_buffer();
  
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
  
  // Topics définis
  std::vector<HeishaTopic> topics_;
  std::vector<HeishaTopic> optional_topics_;
  
  // Predefined queries (ported from HeishaMon)
  std::vector<uint8_t> initial_query_;
  std::vector<uint8_t> panasonic_query_;
  std::vector<uint8_t> optional_pcb_query_;
};

}  // namespace heishamon
}  // namespace esphome
