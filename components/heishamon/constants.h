#pragma once

namespace esphome {
namespace heishamon {

// HeishaMon query types
enum class QueryType : uint8_t {
  INITIAL = 0x31,
  PANASONIC_NORMAL = 0x71,
  PANASONIC_EXTRA = 0x71,  // with byte[3] = 0x21
  OPTIONAL_PCB = 0xF1
};

// Types de réponses
enum class ResponseType : uint8_t {
  NORMAL_DATA = 0x10,
  EXTRA_DATA = 0x21,
  OPTIONAL_DATA = 0x50
};

// Modes d'opération (valeurs de decode.cpp)
enum class OperationMode : uint8_t {
  HEAT = 18,
  COOL = 19,
  AUTO_HEAT = 25,
  DHW = 33,
  HEAT_DHW = 34,
  COOL_DHW = 35,
  AUTO_HEAT_DHW = 41,
  AUTO_COOL = 26,
  AUTO_COOL_DHW = 42
};

// Index of important bytes in data (from decode.h)
namespace DataIndex {
  constexpr uint8_t HEATPUMP_STATE = 4;
  constexpr uint8_t OPERATION_MODE = 6;
  constexpr uint8_t DHW_TARGET_TEMP = 42;
  constexpr uint8_t DHW_TEMP = 141;
  constexpr uint8_t OUTSIDE_TEMP = 142;
  constexpr uint8_t MAIN_INLET_TEMP = 143;
  constexpr uint8_t MAIN_OUTLET_TEMP = 144;
  constexpr uint8_t MAIN_TARGET_TEMP = 153;
  constexpr uint8_t COMPRESSOR_FREQ = 166;
  constexpr uint8_t PUMP_FLOW_HIGH = 170;
  constexpr uint8_t PUMP_FLOW_LOW = 169;
  constexpr uint8_t HEAT_POWER_CONSUMPTION = 193;
  constexpr uint8_t HEAT_POWER_PRODUCTION = 194;
}

// Valeurs par défaut
namespace Defaults {
  constexpr uint32_t UPDATE_INTERVAL_MS = 30000;
  constexpr uint32_t OPTIONAL_PCB_INTERVAL_MS = 1000;
  constexpr uint32_t SERIAL_TIMEOUT_MS = 2000;
  constexpr float UNKNOWN_VALUE = -1.0f;
}

// Data validation
namespace Validation {
  constexpr float MIN_TEMPERATURE = -50.0f;
  constexpr float MAX_TEMPERATURE = 100.0f;
  constexpr float MIN_POWER = 0.0f;
  constexpr float MAX_POWER = 20000.0f;  // 20kW max réaliste
  constexpr float MIN_FREQUENCY = 0.0f;
  constexpr float MAX_FREQUENCY = 200.0f;  // 200Hz max for compressor
}

}  // namespace heishamon
}  // namespace esphome
