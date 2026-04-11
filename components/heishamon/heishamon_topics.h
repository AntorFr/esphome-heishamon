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
 * Command encoding types - how the input value is transformed before writing to the send buffer
 */
enum class CommandEncoding : uint8_t {
  VALUE_PLUS_128,        // byte = value + 128 (temperatures, signed values)
  VALUE_PLUS_1,          // byte = value + 1 (unsigned values like pump duty, heater delay)
  POWER_PLUS_128,        // byte = (value + 1) * 200 style → actually value + 128 for power
  ONOFF_64_128,          // 0→64, 1→128 (force DHW, main schedule, etc.)
  ONOFF_16_32,           // 0→16, 1→32 (holiday, pump, alt sensor, etc.)
  ONOFF_1_2,             // 0→1, 1→2 (heatpump state, bivalent control, external control)
  ONOFF_0_2,             // 0→0, 1→2 (force defrost)
  ONOFF_0_4,             // 0→0, 1→4 (force sterilization)
  QUIET_MODE,            // byte = (value + 1) * 8
  POWERFUL_MODE,         // byte = (value + 1) & 0b111
  OPERATION_MODE,        // Special map: 0→18, 1→19, 2→24, 3→33, 4→34, 5→35, 6→40
  ZONES,                 // 0→64, 1→128, 2→192
  PAD_HEATER,            // 0→16, 1→32, 2→48
  BIVALENT_MODE,         // 0→4, 1→8, 2→12
  BIT_SHIFT_2,           // byte = (value+1) << 2 (heating control, dhw heater state)
  BIT_SHIFT_4,           // byte = (value+1) << 4 (quiet mode priority, pump flowrate)
  BIT_SHIFT_6,           // byte = (value+1) << 6 (smart DHW)
  DIRECT,                // byte = value (direct, for DHW sensor selection, room heater state)
  ONOFF_4_8,             // 0→4, 1→8 (buffer, external heat/cool control)
  ONOFF_16_32_48,        // alias for PAD_HEATER
  EXTERNAL_ERROR,        // 0→16, 1→32
  EXTERNAL_COMPRESSOR,   // 0→64, 1→128
};

/**
 * Command configuration structure
 * Maps command names to their protocol byte position and encoding
 * Based on the 110-byte panasonicSendQuery with header {0xF1, 0x6C, 0x01, 0x10}
 */
struct CommandConfig {
  std::string name;                             // Command name (e.g., "SetDHWTemp")
  uint8_t byte_position;                       // Position in 110-byte send buffer
  CommandEncoding encoding;                     // How to encode the value
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
  
  // Command encoding - build a 110-byte send buffer with the appropriate value
  static std::vector<uint8_t> encode_command(const CommandConfig &config, int value);
  static uint8_t encode_value(CommandEncoding encoding, int value);
  
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
                                     uint8_t byte_position,
                                     CommandEncoding encoding,
                                     const std::string &description = "");
};

}  // namespace heishamon
}  // namespace esphome