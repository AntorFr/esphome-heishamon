# HeishaMon ESPHome Debugging Guide

## Log Configuration

To diagnose issues, enable detailed logging:

```yaml
logger:
  level: DEBUG
  logs:
    heishamon: DEBUG
    heishamon.sensor: DEBUG
    heishamon.binary_sensor: DEBUG
    heishamon.switch: DEBUG
    uart: DEBUG
```

## Typical Log Messages

### Normal Startup
```
[heishamon:xxx] Setting up Heishamon...
[heishamon:xxx] Heishamon setup completed
[heishamon.sensor:xxx] Setting up Heishamon Sensor 'main_inlet_temp'...
```

### Normal Communication
```
[heishamon:xxx] Sending panasonic query
[heishamon:xxx] Received valid packet, size: 203
[heishamon.sensor:xxx] main_inlet_temp: 22.5°C
```

### Communication Issues

#### No Data Received
```
[heishamon:xxx] Command timeout, resetting send state
```
**Solutions:**
- Check TX/RX connections
- Verify level shifter
- Check power supply

#### Invalid Header
```
[heishamon:xxx] Invalid header: 0xXX
```
**Solutions:**
- Check serial connections
- Check for electromagnetic interference
- Add filter capacitors

#### Invalid Checksum
```
[heishamon:xxx] Invalid checksum
```
**Solutions:**
- Check connection quality
- Move away from interference sources
- Verify data integrity

### Listen-Only Mode with CZ-TAW1

If using `listen_only: true` with a CZ-TAW1:
```
[heishamon:xxx] Cannot send command in listen-only mode
```
This is normal, commands are disabled in listen-only mode.

## Diagnostic Tools

### 1. Hardware Verification

```yaml
# Add temporarily to test GPIO
output:
  - platform: gpio
    pin: GPIO15
    id: test_tx
  - platform: gpio  
    pin: GPIO13
    id: test_rx

switch:
  - platform: output
    name: "Test TX"
    output: test_tx
  - platform: output
    name: "Test RX" 
    output: test_rx
```

### 2. ESPHome Log Monitor

Use ESPHome command to see real-time logs:
```bash
esphome logs example-esp8266.yaml
```

### 3. Frame Analysis

Temporarily add to code to see raw data:
```cpp
// In heishamon.cpp, read_serial() method
ESP_LOGD(TAG, "Raw data: %s", format_hex_pretty(this->data_buffer_).c_str());
```

## Common Issues and Solutions

### 1. ESP Restarts in Loop
**Cause:** Power supply issues or GPIO conflicts
**Solution:**
- Verify stable 3.3V power supply
- Change GPIO pins used
- Add decoupling capacitors

### 2. Erratic Data
**Cause:** Interference or poor connections
**Solution:**
- Use shielded cables
- Add ferrite cores
- Move away from RF/WiFi sources

### 3. CZ-TAW1 Conflicts
**Cause:** Two devices trying to communicate simultaneously
**Solution:**
- Enable `listen_only: true`
- Verify parallel connections
- Do not use enable pin with CZ-TAW1

### 4. Incorrect Values
**Cause:** Wrong decoding or different protocol version
**Solution:**
- Check heat pump model
- Compare with original HeishaMon logs
- Adjust decoding functions if necessary

## Validation Tests

### Test 1: Basic Communication
1. Configure with `listen_only: false`
2. Verify data reception every 30s
3. Check temperature values are reasonable

### Test 2: Listen-Only Mode  
1. Configure with `listen_only: true`
2. Verify commands are rejected
3. Check data reception continues

### Test 3: Commands
1. Configure a switch
2. Test sending a simple command (e.g., force_dhw)
3. Verify heat pump responds

## ESPHome Compilation Tests

### Automatic Validation
Use the provided script to validate configurations:
```bash
./test-compilation.sh
```

### Manual Validation
```bash
# Environment setup
python3 -m venv .venv
source .venv/bin/activate
pip install esphome

# Test ESP8266
esphome config example-esp8266.yaml

# Test ESP32  
esphome config example-esp32.yaml

# Full compilation (optional)
esphome compile example-esp8266.yaml
esphome compile example-esp32.yaml
```

### Tested Versions
- **ESPHome**: 2025.7.4 and newer
- **ESP8266 Arduino Core**: 3.1.2
- **ESP32 Arduino Core**: 3.1.3
- **Python**: 3.8+ recommended

### Common Compilation Errors

#### Missing Unit Constants
```
ImportError: cannot import name 'UNIT_LITER_PER_MINUTE'
```
**Solution**: Update to ESPHome 2025.7.4+ and use `UNIT_CUBIC_METER_PER_HOUR`

#### Obsolete Switch API
```
TypeError: switch_schema() missing 1 required positional argument: 'class_'
```
**Solution**: Use `switch.switch_schema(HeishamonSwitch)` instead of `switch.switch_schema()`

#### Missing OTA Platform
```
At least one platform must be specified for 'ota'
```
**Solution**: Add `platform: esphome` in the `ota:` section

#### Obsolete ESP Platform
```
Please remove the `platform` key from the [esphome] block
```
**Solution**: Use `esp8266:` or `esp32:` instead of `platform:` in `esphome:`

## System Information

### Memory Requirements
- ESP8266: ~20KB RAM, ~100KB Flash
- ESP32: ~15KB RAM, ~120KB Flash

### Performance
- Recommended update interval: 30s minimum
- Packet processing time: <10ms
- Command latency: <100ms

## Support and Help

1. **GitHub Issues**: Open a ticket with complete logs
2. **ESPHome Forum**: Post in custom components section  
3. **HeishaMon Discord**: #esphome-port channel

Always include:
- Complete YAML configuration
- Logs with DEBUG level
- Heat pump model
- Wiring diagram used
