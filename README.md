# ESPHome HeishaMon Component

This is an ESPHome custom component that ports HeishaMon functionality for Panasonic heat pump communication.

## Overview

HeishaMon is a popular open-source project for monitoring and controlling Panasonic heat pumps. This ESPHome component provides the essential communication features, while leveraging ESPHome's built-in capabilities for WiFi, MQTT, web server, and Home Assistant integration.

## Features

- **Serial Communication**: Full protocol implementation compatible with original HeishaMon
- **Sensor Support**: Temperature, power, frequency, and flow sensors
- **Binary Sensors**: Heat pump state monitoring
- **Switch Controls**: Force DHW, defrost, holiday mode commands
- **Listen-Only Mode**: Compatible with existing CZ-TAW1 installations
- **Platform Support**: ESP8266 and ESP32 with optimized configurations
- **Home Assistant**: Native integration via ESPHome API

## Quick Start

1. Copy the `components/` folder to your ESPHome project
2. Use the configuration example provided in `example-esp8266.yaml` or `example-esp32.yaml`
3. Connect your ESP to the heat pump's CN-CNT connector via level shifter
4. Flash and enjoy!

## Project Structure

```
esphome-heishamon/
├── components/
│   └── heishamon/
│       ├── __init__.py           # ESPHome configuration
│       ├── heishamon.h           # Main C++ header
│       ├── heishamon.cpp         # Main implementation
│       ├── sensor.py             # Sensor configuration
│       ├── sensor.h              # Sensor header
│       ├── sensor.cpp            # Sensor implementation
│       ├── binary_sensor.py      # Binary sensor configuration
│       ├── binary_sensor.h       # Binary sensor header
│       ├── binary_sensor.cpp     # Binary sensor implementation
│       ├── switch.py             # Switch configuration
│       ├── switch.h              # Switch header
│       └── switch.cpp            # Switch implementation
├── docs/
│   ├── WIRING.md                # Hardware wiring diagrams
│   └── DEBUGGING.md             # Troubleshooting guide
├── example-esp8266.yaml         # ESP8266 configuration example
├── example-esp32.yaml           # ESP32 configuration example
└── README.md                    # Main documentation
```

## Hardware Setup

For detailed wiring diagrams and hardware setup instructions, see **[`docs/WIRING.md`](docs/WIRING.md)**.

### Basic Connection

### ESP8266 vs ESP32 Comparison

| Function | ESP8266 (Wemos D1 Mini) | ESP32 (ESP32-DEV) | Description |
|----------|--------------------------|-------------------|-------------|
| **TX** | GPIO15 (D8) | GPIO17 | Transmission to heat pump |
| **RX** | GPIO13 (D7) | GPIO18 | Reception from heat pump |
| **Enable** | GPIO5 (D1) | GPIO5 | Level shifter control (optional) |
| **OT Enable** | - | GPIO4 | OpenTherm control (ESP32 only) |
| **Status LED** | GPIO2 (D4) | GPIO2 | Status LED (optional) |
| **Power** | 3.3V/5V via CN-CNT | 3.3V/5V via CN-CNT | Power from heat pump |

### Wemos D1 Mini (ESP8266)
- **TX** (GPIO15 / D8) → CN-CNT pin 2 (TX) via level shifter
- **RX** (GPIO13 / D7) → CN-CNT pin 3 (RX) via level shifter  
- **Enable** (GPIO5 / D1) → Level shifter control (optional)
- **Power** 5V from CN-CNT pin 1 and 4

### ESP32 Development Board
- **TX** (GPIO17) → CN-CNT pin 2 (TX) via level shifter
- **RX** (GPIO18) → CN-CNT pin 3 (RX) via level shifter
- **Enable** (GPIO5) → Level shifter control (optional)
- **OT Enable** (GPIO4) → OpenTherm control (optional)

### ESP32 Advantages
- More RAM and Flash memory
- Serial proxy support (secondary UART)
- Dedicated OpenTherm GPIO
- Superior performance
- Bluetooth support (for debugging)

**Important:** A 3.3V ↔ 5V level shifter is required as the heat pump uses 5V TTL.

## Configuration Examples

The project includes several configuration examples:

### `example-esp8266.yaml` - ESP8266 Configuration (Wemos D1 Mini)
Optimized configuration for ESP8266 with:
- GPIO15/13 for serial communication
- Listen-only mode by default (recommended)
- Essential sensors only to save memory
- Basic WiFi and API configuration

### `example-esp32.yaml` - ESP32 Configuration
Advanced configuration for ESP32 with:
- GPIO17/18 for main serial communication
- Serial proxy support for monitoring (optional)
- All available sensors
- Advanced features (OpenTherm, status LED)
- ESP32 internal temperature

## ESPHome Configuration

### Basic UART Configuration

```yaml
uart:
  id: heisha_uart
  tx_pin: GPIO15  # D8 for Wemos D1 Mini
  rx_pin: GPIO13  # D7 for Wemos D1 Mini
  baud_rate: 9600
  parity: EVEN
  stop_bits: 1
```

### Main Component

```yaml
heishamon:
  id: heisha_main
  uart_id: heisha_uart
  update_interval: 30s    # Interval between requests (default: 30s)
  listen_only: false      # Listen-only mode (default: false)
  optional_pcb: false     # Optional PCB support (default: false)
```

#### Configuration Parameters

- **update_interval**: Interval between data requests (recommended: 30s minimum)
- **listen_only**: 
  - `true`: Listen-only mode, sends no commands (use if CZ-TAW1 is connected)
  - `false`: Normal mode, can send commands
- **optional_pcb**: Support for optional PCB data (additional zones, etc.)

### Available Sensors

| Topic | Description | Unit | Type |
|-------|-------------|------|------|
| `main_inlet_temp` | Main inlet temperature | °C | sensor |
| `main_outlet_temp` | Main outlet temperature | °C | sensor |
| `main_target_temp` | Main target temperature | °C | sensor |
| `dhw_temp` | DHW temperature | °C | sensor |
| `dhw_target_temp` | DHW target temperature | °C | sensor |
| `outside_temp` | Outside temperature | °C | sensor |
| `compressor_freq` | Compressor frequency | Hz | sensor |
| `heat_power_production` | Heat power production | W | sensor |
| `heat_power_consumption` | Heat power consumption | W | sensor |
| `pump_flow` | Pump flow | m³/h | sensor |
| `operation_mode` | Operation mode | - | sensor |

### Available Binary Sensors

| Topic | Description | Device Class |
|-------|-------------|--------------|
| `heatpump_state` | Heat pump running state | running |

### Available Switches

| Command | Description |
|---------|-------------|
| `force_dhw` | Force DHW heating |
| `force_defrost` | Force defrost cycle |
| `holiday_mode` | Enable/disable holiday mode |
| `heatpump_state` | Turn heat pump on/off |

See the files `example-esp8266.yaml` and `example-esp32.yaml` for complete configurations.

## Protocol Details

### Communication Specifications
- **Baud Rate**: 9600
- **Parity**: EVEN
- **Stop Bits**: 1
- **Data Bits**: 8

### Message Format
- **Header**: 0x71 (normal), 0xF1 (optional PCB), 0x31 (init)
- **Data Size**: 203 bytes (normal), 20 bytes (optional PCB)
- **Checksum**: XOR of all bytes + 1

### Supported Heat Pump Models
- Panasonic Aquarea series (J, K, L, M, P generations)
- Compatible with CN-CNT connector
- Tested with various models from 2012-2024

## Compatibility

### With CZ-TAW1
If you have a CZ-TAW1 controller installed:
1. Set `listen_only: true` in configuration
2. Connect in parallel to CN-CNT (do not use enable pin)
3. Commands will be disabled to avoid conflicts

### Memory Requirements
- **ESP8266**: ~20KB RAM, ~100KB Flash
- **ESP32**: ~15KB RAM, ~120KB Flash

### Performance
- Update interval: 30s minimum recommended
- Packet processing time: <10ms
- Command latency: <100ms

## Troubleshooting

For detailed troubleshooting information, see **[`docs/DEBUGGING.md`](docs/DEBUGGING.md)**.

### Common Issues

1. **No data received**: Check TX/RX connections and level shifter
2. **Invalid checksums**: Check for electromagnetic interference
3. **ESP restarts**: Verify power supply and GPIO conflicts
4. **Erratic data**: Use shielded cables and ferrites

## License

This project is based on the original HeishaMon by Egyras:
https://github.com/Egyras/HeishaMon

## Support

- **GitHub Issues**: Open a ticket with complete logs
- **ESPHome Forum**: Post in custom components section
- **HeishaMon Discord**: #esphome-port channel

Always include:
- Complete YAML configuration
- Logs with DEBUG level
- Heat pump model
- Wiring diagram used
