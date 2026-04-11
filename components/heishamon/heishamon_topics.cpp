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
                                             uint8_t byte_position,
                                             CommandEncoding encoding,
                                             const std::string &description) {
  return {name, byte_position, encoding, description};
}

std::vector<TopicConfig> HeishamonTopics::get_standard_topics() {
  return {
    // Phase 1: Essential temperature sensors
    create_topic("dhw_temp", 141, get_int_minus_128, "°C", "DHW Tank Temperature", false, "temperature"),
    create_topic("dhw_target_temp", 42, get_int_minus_128, "°C", "DHW Target Temperature", false, "temperature"),
    create_topic("outside_temp", 142, get_int_minus_128, "°C", "Outside Temperature", false, "temperature"),
    create_topic("main_inlet_temp", 143, get_int_minus_128, "°C", "Main Inlet Temperature", false, "temperature"),
    create_topic("main_outlet_temp", 144, get_int_minus_128, "°C", "Main Outlet Temperature", false, "temperature"),
    create_topic("main_target_temp", 153, get_int_minus_128, "°C", "Main Target Temperature", false, "temperature"),
    
    // Zone temperatures
    create_topic("z1_temp", 139, get_int_minus_128, "°C", "Zone 1 Temperature", false, "temperature"),
    create_topic("z2_temp", 140, get_int_minus_128, "°C", "Zone 2 Temperature", false, "temperature"),
    
    // Zone target temperatures (for number components)
    create_topic("z1_heat_target_temp", 38, get_int_minus_128, "°C", "Zone 1 Heat Target Temperature", false, "temperature"),
    create_topic("z1_cool_target_temp", 39, get_int_minus_128, "°C", "Zone 1 Cool Target Temperature", false, "temperature"),
    create_topic("z2_heat_target_temp", 40, get_int_minus_128, "°C", "Zone 2 Heat Target Temperature", false, "temperature"),
    create_topic("z2_cool_target_temp", 41, get_int_minus_128, "°C", "Zone 2 Cool Target Temperature", false, "temperature"),
    
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
    // --- Heatpump ON/OFF ---
    create_command("SetHeatpump", 4, CommandEncoding::ONOFF_1_2, "Set heatpump on/off"),
    create_command("SetPump", 4, CommandEncoding::ONOFF_16_32, "Set pump on/off"),
    
    // --- Operation modes ---
    create_command("SetOperationMode", 6, CommandEncoding::OPERATION_MODE, "Set operation mode"),
    create_command("SetZones", 6, CommandEncoding::ZONES, "Set active zones"),
    
    // --- Quiet/Powerful modes ---
    create_command("SetQuietMode", 7, CommandEncoding::QUIET_MODE, "Set quiet mode level"),
    create_command("SetPowerfulMode", 7, CommandEncoding::POWERFUL_MODE, "Set powerful mode time"),
    
    // --- Holiday mode ---
    create_command("SetHolidayMode", 5, CommandEncoding::ONOFF_16_32, "Set holiday mode"),
    
    // --- Main schedule ---
    create_command("SetMainSchedule", 5, CommandEncoding::ONOFF_64_128, "Set main schedule on/off"),
    
    // --- Force operations ---
    create_command("SetForceDHW", 4, CommandEncoding::ONOFF_64_128, "Force DHW heating"),
    create_command("SetForceDefrost", 8, CommandEncoding::ONOFF_0_2, "Force defrost"),
    create_command("SetForceSterilization", 8, CommandEncoding::ONOFF_0_4, "Force sterilization"),
    create_command("SetReset", 8, CommandEncoding::DIRECT, "Reset error"),
    
    // --- Zone temperature requests ---
    create_command("SetZ1HeatRequestTemperature", 38, CommandEncoding::VALUE_PLUS_128, "Set Z1 heat request temp"),
    create_command("SetZ1CoolRequestTemperature", 39, CommandEncoding::VALUE_PLUS_128, "Set Z1 cool request temp"),
    create_command("SetZ2HeatRequestTemperature", 40, CommandEncoding::VALUE_PLUS_128, "Set Z2 heat request temp"),
    create_command("SetZ2CoolRequestTemperature", 41, CommandEncoding::VALUE_PLUS_128, "Set Z2 cool request temp"),
    
    // --- DHW temperature ---
    create_command("SetDHWTemp", 42, CommandEncoding::VALUE_PLUS_128, "Set DHW target temperature"),
    
    // --- Max pump duty ---
    create_command("SetMaxPumpDuty", 45, CommandEncoding::VALUE_PLUS_1, "Set max pump duty"),
    
    // --- Buffer tank delta ---
    create_command("SetBufferDelta", 59, CommandEncoding::VALUE_PLUS_128, "Set buffer tank delta"),
    
    // --- Curve temperatures (Z1 Heat) ---
    // NOTE: SetCurves in the original is a JSON command setting multiple bytes.
    // We implement individual curve setters instead.
    create_command("SetZ1HeatCurveTargetHigh", 75, CommandEncoding::VALUE_PLUS_128, "Set Z1 heat curve target high"),
    create_command("SetZ1HeatCurveTargetLow", 76, CommandEncoding::VALUE_PLUS_128, "Set Z1 heat curve target low"),
    create_command("SetZ1HeatCurveOutsideLow", 77, CommandEncoding::VALUE_PLUS_128, "Set Z1 heat curve outside low"),
    create_command("SetZ1HeatCurveOutsideHigh", 78, CommandEncoding::VALUE_PLUS_128, "Set Z1 heat curve outside high"),
    
    // --- Curve temperatures (Z2 Heat) ---
    create_command("SetZ2HeatCurveTargetHigh", 79, CommandEncoding::VALUE_PLUS_128, "Set Z2 heat curve target high"),
    create_command("SetZ2HeatCurveTargetLow", 80, CommandEncoding::VALUE_PLUS_128, "Set Z2 heat curve target low"),
    create_command("SetZ2HeatCurveOutsideLow", 81, CommandEncoding::VALUE_PLUS_128, "Set Z2 heat curve outside low"),
    create_command("SetZ2HeatCurveOutsideHigh", 82, CommandEncoding::VALUE_PLUS_128, "Set Z2 heat curve outside high"),
    
    // --- Heating off outdoor temp ---
    create_command("SetHeatingOffOutdoorTemp", 83, CommandEncoding::VALUE_PLUS_128, "Set heating off outdoor temp"),
    
    // --- Floor/DHW deltas ---
    create_command("SetFloorHeatDelta", 84, CommandEncoding::VALUE_PLUS_128, "Set floor heat delta"),
    create_command("SetHeaterOnOutdoorTemp", 85, CommandEncoding::VALUE_PLUS_128, "Set heater on outdoor temp"),
    
    // --- Curve temperatures (Z1 Cool) ---
    create_command("SetZ1CoolCurveTargetHigh", 86, CommandEncoding::VALUE_PLUS_128, "Set Z1 cool curve target high"),
    create_command("SetZ1CoolCurveTargetLow", 87, CommandEncoding::VALUE_PLUS_128, "Set Z1 cool curve target low"),
    create_command("SetZ1CoolCurveOutsideLow", 88, CommandEncoding::VALUE_PLUS_128, "Set Z1 cool curve outside low"),
    create_command("SetZ1CoolCurveOutsideHigh", 89, CommandEncoding::VALUE_PLUS_128, "Set Z1 cool curve outside high"),
    
    // --- Curve temperatures (Z2 Cool) ---
    create_command("SetZ2CoolCurveTargetHigh", 90, CommandEncoding::VALUE_PLUS_128, "Set Z2 cool curve target high"),
    create_command("SetZ2CoolCurveTargetLow", 91, CommandEncoding::VALUE_PLUS_128, "Set Z2 cool curve target low"),
    create_command("SetZ2CoolCurveOutsideLow", 92, CommandEncoding::VALUE_PLUS_128, "Set Z2 cool curve outside low"),
    create_command("SetZ2CoolCurveOutsideHigh", 93, CommandEncoding::VALUE_PLUS_128, "Set Z2 cool curve outside high"),
    
    // --- Floor cool delta ---
    create_command("SetFloorCoolDelta", 94, CommandEncoding::VALUE_PLUS_128, "Set floor cool delta"),
    
    // --- DHW heat delta ---
    create_command("SetDHWHeatDelta", 99, CommandEncoding::VALUE_PLUS_128, "Set DHW heat delta"),
    
    // --- Heater timing ---
    create_command("SetHeaterDelayTime", 104, CommandEncoding::VALUE_PLUS_1, "Set heater delay time"),
    create_command("SetHeaterStartDelta", 105, CommandEncoding::VALUE_PLUS_128, "Set heater start delta"),
    create_command("SetHeaterStopDelta", 106, CommandEncoding::VALUE_PLUS_128, "Set heater stop delta"),
    
    // --- DHW sensor selection (byte 11) ---
    create_command("SetDHWSensorSelection", 11, CommandEncoding::DIRECT, "Set DHW sensor selection"),
    create_command("SetQuietModePriority", 11, CommandEncoding::BIT_SHIFT_4, "Set quiet mode priority"),
    
    // --- DHW/Room heater state (byte 9) ---
    create_command("SetDHWHeaterState", 9, CommandEncoding::BIT_SHIFT_2, "Set DHW heater state"),
    create_command("SetRoomHeaterState", 9, CommandEncoding::DIRECT, "Set room heater state"),
    
    // --- Config byte 20 ---
    create_command("SetAltExternalSensor", 20, CommandEncoding::ONOFF_16_32, "Set alt external sensor"),
    
    // --- Config byte 23 (external controls) ---
    create_command("SetExternalControl", 23, CommandEncoding::ONOFF_1_2, "Set external control"),
    create_command("SetExternalHeatCoolControl", 23, CommandEncoding::ONOFF_4_8, "Set external heat/cool control"),
    create_command("SetExternalError", 23, CommandEncoding::EXTERNAL_ERROR, "Set external error signal"),
    create_command("SetExternalCompressorControl", 23, CommandEncoding::EXTERNAL_COMPRESSOR, "Set external compressor control"),
    
    // --- Config byte 24 ---
    create_command("SetBuffer", 24, CommandEncoding::ONOFF_4_8, "Set buffer enabled"),
    create_command("SetSmartDHW", 24, CommandEncoding::BIT_SHIFT_6, "Set smart DHW"),
    
    // --- Config byte 25 ---
    create_command("SetExternalPadHeater", 25, CommandEncoding::PAD_HEATER, "Set external pad heater"),
    
    // --- Config byte 26 (bivalent) ---
    create_command("SetBivalentControl", 26, CommandEncoding::ONOFF_1_2, "Set bivalent control on/off"),
    create_command("SetBivalentMode", 26, CommandEncoding::BIVALENT_MODE, "Set bivalent mode"),
    
    // --- Heating mode byte 28 ---
    // (read-only, decoded by getBit7and8 - no set command in original)
    
    // --- Pump flowrate mode byte 29 ---
    create_command("SetPumpFlowrateMode", 29, CommandEncoding::BIT_SHIFT_4, "Set pump flowrate mode"),
    
    // --- Heating control byte 30 ---
    create_command("SetHeatingControl", 30, CommandEncoding::BIT_SHIFT_2, "Set heating control"),
    
    // --- Bivalent temperatures ---
    create_command("SetBivalentStartTemp", 65, CommandEncoding::VALUE_PLUS_128, "Set bivalent start temp"),
    create_command("SetBivalentAPStartTemp", 66, CommandEncoding::VALUE_PLUS_128, "Set bivalent AP start temp"),
    create_command("SetBivalentAPStopTemp", 68, CommandEncoding::VALUE_PLUS_128, "Set bivalent AP stop temp"),
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
  return {"", 0, CommandEncoding::DIRECT, ""};
}

uint8_t HeishamonTopics::encode_value(CommandEncoding encoding, int value) {
  switch (encoding) {
    case CommandEncoding::VALUE_PLUS_128:
      return static_cast<uint8_t>(value + 128);
    case CommandEncoding::VALUE_PLUS_1:
      return static_cast<uint8_t>(value + 1);
    case CommandEncoding::ONOFF_64_128:
      return (value == 0) ? 64 : 128;
    case CommandEncoding::ONOFF_16_32:
      return (value == 0) ? 16 : 32;
    case CommandEncoding::ONOFF_1_2:
      return (value == 0) ? 1 : 2;
    case CommandEncoding::ONOFF_0_2:
      return (value == 0) ? 0 : 2;
    case CommandEncoding::ONOFF_0_4:
      return (value == 0) ? 0 : 4;
    case CommandEncoding::ONOFF_4_8:
      return (value == 0) ? 4 : 8;
    case CommandEncoding::QUIET_MODE:
      // 0→8 (off), 1→16, 2→24, 3→32
      return static_cast<uint8_t>((value + 1) * 8);
    case CommandEncoding::POWERFUL_MODE:
      // 0→off(0), 30min→1, 60min→2, 90min→3
      // In original: (value + 1) stored in bits 0-2, but 0 means off
      // Original: panasonicSendQuery[7] = (value + 1) & 0b111 — but only bits 0-2
      return static_cast<uint8_t>((value + 1) & 0b111);
    case CommandEncoding::OPERATION_MODE:
      // Map: 0→Heat(18), 1→Cool(19), 2→Auto/Heat(24), 3→DHW(33),
      //       4→Heat+DHW(34), 5→Cool+DHW(35), 6→Auto/Heat+DHW(40),
      //       7→Auto/Cool(26), 8→Auto/Cool+DHW(42)
      {
        static const uint8_t mode_map[] = {18, 19, 24, 33, 34, 35, 40, 26, 42};
        if (value >= 0 && value <= 8)
          return mode_map[value];
        return 0;
      }
    case CommandEncoding::ZONES:
      // 0→Zone1 only(64), 1→Zone2 only(128), 2→Both(192)
      {
        static const uint8_t zone_map[] = {64, 128, 192};
        if (value >= 0 && value <= 2)
          return zone_map[value];
        return 64;
      }
    case CommandEncoding::PAD_HEATER:
    case CommandEncoding::ONOFF_16_32_48:
      // 0→disabled(16), 1→type-A(32), 2→type-B(48)
      return static_cast<uint8_t>((value + 1) * 16);
    case CommandEncoding::BIVALENT_MODE:
      // 0→Eco(4), 1→Comfort(8), 2→Balanced(12)
      return static_cast<uint8_t>((value + 1) * 4);
    case CommandEncoding::BIT_SHIFT_2:
      return static_cast<uint8_t>((value + 1) << 2);
    case CommandEncoding::BIT_SHIFT_4:
      return static_cast<uint8_t>((value + 1) << 4);
    case CommandEncoding::BIT_SHIFT_6:
      return static_cast<uint8_t>((value + 1) << 6);
    case CommandEncoding::EXTERNAL_ERROR:
      return (value == 0) ? 16 : 32;
    case CommandEncoding::EXTERNAL_COMPRESSOR:
      return (value == 0) ? 64 : 128;
    case CommandEncoding::DIRECT:
    default:
      return static_cast<uint8_t>(value);
  }
}

std::vector<uint8_t> HeishamonTopics::encode_command(const CommandConfig &config, int value) {
  // Build a 110-byte send buffer with header {0xF1, 0x6C, 0x01, 0x10}
  std::vector<uint8_t> command(110, 0);
  command[0] = 0xF1;
  command[1] = 0x6C;
  command[2] = 0x01;
  command[3] = 0x10;
  
  // Encode and set the value at the correct position
  command[config.byte_position] = encode_value(config.encoding, value);
  
  return command;
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