<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# ESPHome HeishaMon Component

This is an ESPHome custom component project that ports HeishaMon functionality for Panasonic heat pump communication.

## Project Structure

- `components/heishamon/` - Main component implementation
- `example-esp8266.yaml` - ESP8266 ESPHome configuration example
- `example-esp32.yaml` - ESP32 ESPHome configuration example
- `README.md` - Complete documentation

## Development Guidelines

1. **ESPHome Compatibility**: All code must be compatible with ESPHome framework
2. **Memory Efficiency**: Optimize for ESP8266/ESP32 memory constraints  
3. **Protocol Accuracy**: Maintain exact protocol compatibility with original HeishaMon
4. **Code Style**: Follow ESPHome coding conventions and C++ best practices

## Key Components

- `heishamon.h/cpp` - Main component with serial communication
- `sensor.h/cpp` - Sensor implementation for numeric values
- `binary_sensor.h/cpp` - Binary sensor for on/off states
- `switch.h/cpp` - Switch implementation for commands
- `__init__.py` - Python configuration schema
- `sensor.py` - Sensor configuration
- `binary_sensor.py` - Binary sensor configuration
- `switch.py` - Switch configuration

## Protocol Details

- **Serial**: 9600 baud, EVEN parity, 1 stop bit
- **Headers**: 0x71 (normal), 0xF1 (optional PCB), 0x31 (init)
- **Checksum**: XOR of all bytes + 1
- **Data size**: 203 bytes (normal), 20 bytes (optional PCB)

## Original HeishaMon Reference

Based on https://github.com/Egyras/HeishaMon - focus on essential communication features only, excluding web server, WiFi config, rules engine, etc. Those features are handled by ESPHome itself.
