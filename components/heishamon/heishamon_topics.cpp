#include "heishamon_topics.h"

namespace esphome {
namespace heishamon {

// Helper function to create topic configs
TopicConfig HeishamonTopics::create_topic(const std::string &name, uint8_t byte_index,
                                         std::function<float(uint8_t)> decode_func,
                                         const std::string &unit,
                                         const std::string &description,
                                         bool is_binary,
                                         const std::string &device_class) {
  return {name, byte_index, decode_func, unit, description, is_binary, device_class};
}

CommandConfig HeishamonTopics::create_command(const std::string &name,
                                             const std::vector<uint8_t> &base_command,
                                             uint8_t value_position,
                                             float min_value, float max_value,
                                             const std::string &description) {
  return {name, base_command, value_position, min_value, max_value, description};
}

std::vector<TopicConfig> HeishamonTopics::get_standard_topics() {
  return {
    // Phase 1: Essential temperature sensors
    create_topic("dhw_temp", 141, get_int_minus_128, "°C", "DHW Tank Temperature", false, "temperature"),
    create_topic("dhw_target_temp", 42, get_int_minus_1, "°C", "DHW Target Temperature", false, "temperature"),
    create_topic("outside_temp", 142, get_int_minus_128, "°C", "Outside Temperature", false, "temperature"),
    create_topic("main_inlet_temp", 143, get_int_minus_128, "°C", "Main Inlet Temperature", false, "temperature"),
    create_topic("main_outlet_temp", 144, get_int_minus_128, "°C", "Main Outlet Temperature", false, "temperature"),
    create_topic("main_target_temp", 153, get_int_minus_128, "°C", "Main Target Temperature", false, "temperature"),
    
    // Zone temperatures
    create_topic("z1_temp", 139, get_int_minus_128, "°C", "Zone 1 Temperature", false, "temperature"),
    create_topic("z2_temp", 140, get_int_minus_128, "°C", "Zone 2 Temperature", false, "temperature"),
    
    // Zone target temperatures (for number components)
    create_topic("z1_heat_target_temp", 38, get_int_minus_1, "°C", "Zone 1 Heat Target Temperature", false, "temperature"),
    create_topic("z1_cool_target_temp", 39, get_int_minus_1, "°C", "Zone 1 Cool Target Temperature", false, "temperature"),
    create_topic("z2_heat_target_temp", 40, get_int_minus_1, "°C", "Zone 2 Heat Target Temperature", false, "temperature"),
    create_topic("z2_cool_target_temp", 41, get_int_minus_1, "°C", "Zone 2 Cool Target Temperature", false, "temperature"),
    
    // Binary sensors - Heat pump states
    create_topic("heatpump_state", 4, get_bit_7_and_8, "", "Heat Pump State", true, "running"),
    create_topic("pump_flow", 169, get_pump_flow_special, "L/min", "Pump Flow", false, ""),
    create_topic("dhw_heating", 112, get_bit_1_and_2, "", "DHW Heating Active", true, "heat"),
    
    // Switch states (read-only for listen_only mode)
    create_topic("force_dhw", 112, get_bit_1_and_2, "", "Force DHW State", true, "switch"),
    create_topic("holiday_mode", 113, get_bit_5_and_6, "", "Holiday Mode State", true, "switch"),
    
    // Power and energy
    create_topic("power", 194, get_power, "W", "Power Consumption", false, "power"),
    create_topic("dhw_power", 195, get_dhw_power, "W", "DHW Power", false, "power"),
    
    // Operating modes and states
    create_topic("operating_mode_state", 113, get_bit_3_and_4, "", "Operating Mode State", false, ""),
    create_topic("zones_state", 114, get_bit_5_and_6, "", "Zones State", false, ""),
    create_topic("dhw_mode", 115, unknown, "", "DHW Mode", false, ""),
    
    // Phase 2: Extended sensors for monitoring
    create_topic("compressor_freq", 166, get_int_minus_1, "Hz", "Compressor Frequency", false, "frequency"),
    create_topic("operation_mode", 6, get_operation_mode, "", "Operation Mode", false, ""),
    create_topic("liquid_temp", 134, get_int_minus_128, "°C", "Liquid Temperature", false, "temperature"),
    create_topic("discharge_temp", 158, get_int_minus_128, "°C", "Discharge Temperature", false, "temperature"),
    create_topic("suction_temp", 159, get_int_minus_128, "°C", "Suction Temperature", false, "temperature"),
    create_topic("heat_delta", 137, get_heat_delta, "°C", "Heat Delta T", false, "temperature"),
    create_topic("cool_delta", 138, get_cool_delta, "°C", "Cool Delta T", false, "temperature"),
    
    // Defrost and error states
    create_topic("defrost_state", 198, unknown, "", "Defrost State", true, "running"),
    create_topic("error_state", 113, get_bit_1_and_2, "", "Error State", true, "problem"),
    
    // Phase 3: Operational data
    create_topic("operating_hours", 180, get_operating_hours, "h", "Operating Hours", false, "duration"),
    create_topic("pump_duty", 173, get_int_minus_1, "%", "Pump Duty", false, ""),
    create_topic("fan_speed", 174, get_int_minus_1, "%", "Fan Speed", false, ""),
    
    // Solar/external data if available
    create_topic("solar_temp", 141, get_int_minus_128, "°C", "Solar Temperature", false, "temperature"),
    create_topic("pool_temp", 136, get_int_minus_128, "°C", "Pool Temperature", false, "temperature"),
  };
}

std::vector<TopicConfig> HeishamonTopics::get_optional_topics() {
  return {
    // Optional PCB data (20 bytes packet)
    create_topic("opt_temp_1", 6, get_int_minus_128, "°C", "Optional Temperature 1", false, "temperature"),
    create_topic("opt_temp_2", 7, get_int_minus_128, "°C", "Optional Temperature 2", false, "temperature"),
    create_topic("opt_state_1", 8, unknown, "", "Optional State 1", true, ""),
    create_topic("opt_state_2", 9, unknown, "", "Optional State 2", true, ""),
  };
}

std::vector<CommandConfig> HeishamonTopics::get_available_commands() {
  return {
    // DHW Commands
    create_command("SetDHWTargetTemp", {0x71, 0x16, 0x01, 0x10, 0x42}, 4, 10, 75, "Set DHW target temperature"),
    create_command("SetDHWMode", {0x71, 0x16, 0x01, 0x10, 0x2D}, 4, 0, 3, "Set DHW mode"),
    
    // Zone heating commands
    create_command("SetZ1HeatTargetTemp", {0x71, 0x16, 0x01, 0x10, 0x38}, 4, 5, 35, "Set Zone 1 heat target temperature"),
    create_command("SetZ1CoolTargetTemp", {0x71, 0x16, 0x01, 0x10, 0x39}, 4, 5, 35, "Set Zone 1 cool target temperature"),
    create_command("SetZ2HeatTargetTemp", {0x71, 0x16, 0x01, 0x10, 0x40}, 4, 5, 35, "Set Zone 2 heat target temperature"),
    create_command("SetZ2CoolTargetTemp", {0x71, 0x16, 0x01, 0x10, 0x41}, 4, 5, 35, "Set Zone 2 cool target temperature"),
    
    // System control commands
    create_command("SetHeatMode", {0x71, 0x16, 0x01, 0x10, 0x2E}, 4, 0, 1, "Enable/disable heating mode"),
    create_command("SetCoolMode", {0x71, 0x16, 0x01, 0x10, 0x2F}, 4, 0, 1, "Enable/disable cooling mode"),
    create_command("SetQuietMode", {0x71, 0x16, 0x01, 0x10, 0x30}, 4, 0, 3, "Set quiet mode level"),
    create_command("SetPowerMode", {0x71, 0x16, 0x01, 0x10, 0x31}, 4, 0, 1, "Set power mode"),
    
    // Force operations
    create_command("ForceDHW", {0x71, 0x16, 0x01, 0x10, 0x2A}, 4, 0, 1, "Force DHW heating"),
    create_command("ForceDefrost", {0x71, 0x16, 0x01, 0x10, 0x32}, 4, 0, 1, "Force defrost"),
    create_command("ForceHeater", {0x71, 0x16, 0x01, 0x10, 0x33}, 4, 0, 1, "Force backup heater"),
  };
}

CommandConfig HeishamonTopics::get_command_config(const std::string &command_name) {
  auto commands = get_available_commands();
  for (const auto &cmd : commands) {
    if (cmd.name == command_name) {
      return cmd;
    }
  }
  // Return empty command if not found
  return {"", {}, 0, 0, 0, ""};
}

// Decoding functions implementation (moved from heishamon.cpp)
float HeishamonTopics::unknown(uint8_t input) { return -1; }

float HeishamonTopics::get_bit_1_and_2(uint8_t input) {
  return (input >> 6) - 1;
}

float HeishamonTopics::get_bit_3_and_4(uint8_t input) {
  return ((input >> 4) & 0b11) - 1;
}

float HeishamonTopics::get_bit_5_and_6(uint8_t input) {
  return ((input >> 2) & 0b11) - 1;
}

float HeishamonTopics::get_bit_7_and_8(uint8_t input) {
  return (input & 0b11);
}

float HeishamonTopics::get_int_minus_1(uint8_t input) {
  return static_cast<float>(input) - 1.0f;
}

float HeishamonTopics::get_int_minus_128(uint8_t input) {
  return static_cast<float>(input) - 128.0f;
}

float HeishamonTopics::get_int_minus_1_div_5(uint8_t input) {
  return (static_cast<float>(input) - 1.0f) / 5.0f;
}

float HeishamonTopics::get_int_minus_1_div_50(uint8_t input) {
  return (static_cast<float>(input) - 1.0f) / 50.0f;
}

float HeishamonTopics::get_int_minus_1_times_10(uint8_t input) {
  return (static_cast<float>(input) - 1.0f) * 10.0f;
}

float HeishamonTopics::get_int_minus_1_times_50(uint8_t input) {
  return (static_cast<float>(input) - 1.0f) * 50.0f;
}

float HeishamonTopics::get_power(uint8_t input) {
  // Power calculation specific to HeishaMon
  return static_cast<float>(input) * 200.0f;
}

float HeishamonTopics::get_dhw_power(uint8_t input) {
  return static_cast<float>(input) * 100.0f;
}

float HeishamonTopics::get_heat_delta(uint8_t input) {
  return (static_cast<float>(input) - 128.0f) / 2.0f;
}

float HeishamonTopics::get_cool_delta(uint8_t input) {
  return (static_cast<float>(input) - 128.0f) / 2.0f;
}

float HeishamonTopics::get_operating_hours(uint8_t input) {
  return static_cast<float>(input) * 100.0f;
}

float HeishamonTopics::get_zone_valve_pid(uint8_t input) {
  return static_cast<float>(input) / 2.55f; // Convert to percentage
}

float HeishamonTopics::get_operation_mode(uint8_t input) {
  uint8_t mode = input & 0b111111;
  switch (mode) {
    case 18: return 0.0f; // Heat
    case 19: return 1.0f; // Cool
    case 25: return 2.0f; // Auto(heat)
    case 33: return 3.0f; // DHW
    case 34: return 4.0f; // Heat+DHW
    case 35: return 5.0f; // Cool+DHW
    case 41: return 6.0f; // Auto(heat)+DHW
    case 26: return 7.0f; // Auto(cool)
    case 42: return 8.0f; // Auto(cool)+DHW
    default: return -1.0f; // Unknown
  }
}

float HeishamonTopics::get_pump_flow_special(uint8_t input) {
  // This is a placeholder - the actual decoding needs 2 bytes (169-170)
  // It will be handled specially in the component
  return static_cast<float>(input);
}

float HeishamonTopics::get_pump_flow(const std::vector<uint8_t> &data, uint8_t start_index) {
  if (start_index + 1 < data.size()) {
    uint16_t flow = (data[start_index] << 8) | data[start_index + 1];
    return static_cast<float>(flow) / 10.0f;
  }
  return -1;
}

float HeishamonTopics::get_cop(const std::vector<uint8_t> &data, uint8_t start_index) {
  if (start_index + 1 < data.size()) {
    uint16_t cop = (data[start_index] << 8) | data[start_index + 1];
    return static_cast<float>(cop) / 100.0f;
  }
  return -1;
}

}  // namespace heishamon
}  // namespace esphome