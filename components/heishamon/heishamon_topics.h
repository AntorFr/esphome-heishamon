#pragma once

#include <vector>
#include <string>
#include <functional>

namespace esphome {
namespace heishamon {

/**
 * Topic configuration structure
 * Contains all information needed to decode a specific data point
 */
struct TopicConfig {
  std::string name;                              // Topic name (e.g., "dhw_temp")
  uint8_t byte_index;                           // Position in data packet
  std::function<float(uint8_t)> decode_func;    // Decoding function
  std::string unit;                             // Unit of measurement
  std::string description;                      // Human readable description
  bool is_binary;                               // Is this a binary sensor?
  std::string device_class;                     // ESPHome device class
};

/**
 * Command configuration structure
 * Maps command names to their protocol implementation
 */
struct CommandConfig {
  std::string name;                             // Command name (e.g., "SetDHWTargetTemp")
  std::vector<uint8_t> base_command;           // Base command bytes
  uint8_t value_position;                       // Position where value goes
  float min_value;                              // Minimum allowed value
  float max_value;                              // Maximum allowed value
  std::string description;                      // Description
};

/**
 * HeishamonTopics - Central configuration for all topics and commands
 * 
 * This class provides static configuration for:
 * - Standard heatpump data topics
 * - Optional PCB topics
 * - Available commands
 * - Decoding functions
 */
class HeishamonTopics {
 public:
  // Topic configuration getters
  static std::vector<TopicConfig> get_standard_topics();
  static std::vector<TopicConfig> get_optional_topics();
  
  // Command configuration
  static std::vector<CommandConfig> get_available_commands();
  static CommandConfig get_command_config(const std::string &command_name);
  
  // Decoding functions (static to be used in topic configs)
  static float unknown(uint8_t input);
  static float get_bit_1_and_2(uint8_t input);
  static float get_bit_3_and_4(uint8_t input);
  static float get_bit_5_and_6(uint8_t input);
  static float get_bit_7_and_8(uint8_t input);
  static float get_int_minus_1(uint8_t input);
  static float get_int_minus_128(uint8_t input);
  static float get_int_minus_1_div_5(uint8_t input);
  static float get_int_minus_1_div_50(uint8_t input);
  static float get_int_minus_1_times_10(uint8_t input);
  static float get_int_minus_1_times_50(uint8_t input);
  static float get_power(uint8_t input);
  static float get_dhw_power(uint8_t input);
  static float get_heat_delta(uint8_t input);
  static float get_cool_delta(uint8_t input);
  static float get_operating_hours(uint8_t input);
  static float get_zone_valve_pid(uint8_t input);
  static float get_operation_mode(uint8_t input);
  static float get_pump_flow_special(uint8_t input); // Special case for pump flow
  
  // Multi-byte decoding functions
  static float get_pump_flow(const std::vector<uint8_t> &data, uint8_t start_index);
  static float get_cop(const std::vector<uint8_t> &data, uint8_t start_index);
  
 private:
  // Internal helper to build topic configs
  static TopicConfig create_topic(const std::string &name, uint8_t byte_index,
                                 std::function<float(uint8_t)> decode_func,
                                 const std::string &unit = "",
                                 const std::string &description = "",
                                 bool is_binary = false,
                                 const std::string &device_class = "");
  
  static CommandConfig create_command(const std::string &name,
                                     const std::vector<uint8_t> &base_command,
                                     uint8_t value_position,
                                     float min_value, float max_value,
                                     const std::string &description = "");
};

}  // namespace heishamon
}  // namespace esphome