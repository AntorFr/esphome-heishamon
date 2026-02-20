# ESPHome HeishaMon Component

Native ESPHome component for Panasonic Aquarea heat pumps, based on the original [HeishaMon](https://github.com/Egyras/HeishaMon) project.

**Minimum required ESPHome version: 2026.2.0**

## Features

- **📡 Full Protocol Support**: Complete HeishaMon protocol implementation
- **🌡️ 100+ Sensors**: Temperature, power, pressure, operating hours, zone data
- **🎛️ Controls**: Operating mode, quiet mode, powerful mode, DHW temperature, zone temperatures
- **🔄 Selects**: Operating mode, quiet mode, powerful mode, zones, heating mode
- **🔢 Numbers**: Temperature setpoints, deltas, bivalent settings
- **� Water Heater**: Native DHW (Domestic Hot Water) control with ECO/HEAT_PUMP/PERFORMANCE modes
- **�🔘 Switches**: Force DHW, quiet mode
- **📊 Binary Sensors**: Heat pump state, defrosting, zone pumps, alarms
- **📝 Text Sensors**: Error codes, heat pump model
- **🔇 Listen-Only Mode**: Compatible with existing CZ-TAW1 installations
- **⚡ Platform Support**: ESP8266 and ESP32

## Quick Start

### 1. Add External Component

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/YOUR_USERNAME/esphome-heishamon
      ref: main
    components: [heishamon]
```

### 2. Configure UART

```yaml
uart:
  id: uart_bus
  tx_pin: GPIO17  # ESP32
  rx_pin: GPIO16  # ESP32
  baud_rate: 9600
  parity: EVEN
  stop_bits: 1
```

### 3. Add HeishaMon Component

```yaml
heishamon:
  id: heisha_main
  uart_id: uart_bus
  update_interval: 30s
  listen_only: false  # true for CN-NMODE / CZ-TAW1 coexistence
  optional_pcb: false
```

### 4. Add Sensors

```yaml
sensor:
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "outside_temp"
    name: "Outside Temperature"

  - platform: heishamon
    heishamon_id: heisha_main
    topic: "dhw_temp"
    name: "DHW Temperature"
```

## Configuration Examples

| File | Description |
|------|-------------|
| [example-minimal.yaml](example-minimal.yaml) | Basic configuration for testing |
| [example-full-featured.yaml](example-full-featured.yaml) | All sensors and controls |
| [example-esp8266.yaml](example-esp8266.yaml) | ESP8266 specific configuration |

## Operating Modes

### Active Mode (CN-CNT)

**Configuration:** `listen_only: false`

Connect to **CN-CNT** port for full bidirectional control:
- ✅ All sensors
- ✅ Temperature controls
- ✅ Mode selection
- ✅ Switch commands
- ⚠️ Incompatible with CZ-TAW1

### Listen-Only Mode (CN-NMODE)

**Configuration:** `listen_only: true`

Connect to **CN-NMODE** port for passive monitoring:
- ✅ All sensors (read-only)
- ✅ Compatible with CZ-TAW1
- ❌ No control commands

## Hardware Wiring

### ESP32

| Function | GPIO | CN-CNT Pin |
|----------|------|------------|
| TX | GPIO17 | Pin 2 (TX) |
| RX | GPIO16 | Pin 3 (RX) |
| GND | GND | Pin 4 |
| 5V | VIN | Pin 1 |

### ESP8266 (Wemos D1 Mini)

| Function | GPIO | CN-CNT Pin |
|----------|------|------------|
| TX | GPIO15 (D8) | Pin 2 (TX) |
| RX | GPIO13 (D7) | Pin 3 (RX) |
| GND | GND | Pin 4 |
| 5V | 5V | Pin 1 |

**⚠️ Important:** A 3.3V ↔ 5V level shifter is required!

## Available Topics

### Sensors (sensor platform)

| Topic | Description | TOP# |
|-------|-------------|------|
| `pump_flow` | Pump Flow | TOP1 |
| `outside_temp` | Outside Temperature | TOP14 |
| `main_inlet_temp` | Main Inlet Temperature | TOP5 |
| `main_outlet_temp` | Main Outlet Temperature | TOP6 |
| `main_target_temp` | Main Target Temperature | TOP7 |
| `dhw_temp` | DHW Temperature | TOP10 |
| `dhw_target_temp` | DHW Target Temperature | TOP9 |
| `compressor_freq` | Compressor Frequency | TOP8 |
| `heat_power_production` | Heat Power Production | TOP15 |
| `heat_power_consumption` | Heat Power Consumption | TOP16 |
| `operations_hours` | Operations Hours | TOP11 |
| `z1_water_temp` | Zone 1 Water Temperature | TOP42 |
| `z2_water_temp` | Zone 2 Water Temperature | TOP43 |

See [example-full-featured.yaml](example-full-featured.yaml) for complete sensor list.

### Binary Sensors (binary_sensor platform)

| Topic | Description | TOP# |
|-------|-------------|------|
| `heatpump_state` | Heat Pump Running | TOP0 |
| `defrosting_state` | Defrosting Active | TOP26 |
| `force_dhw_state` | Force DHW Active | TOP2 |
| `internal_heater_state` | Internal Heater | TOP60 |
| `external_heater_state` | External Heater | TOP61 |
| `z1_pump_state` | Zone 1 Pump | TOP124 |
| `z2_pump_state` | Zone 2 Pump | TOP123 |
| `alarm_state` | Alarm Active | TOP44 |

### Selects (select platform)

| Type | Description | TOP# |
|------|-------------|------|
| `operating_mode` | Heat/Cool/Auto/DHW | TOP4 |
| `quiet_mode` | Off/Level 1/2/3/Timer | TOP18 |
| `powerful_mode` | Off/30/60/90 min | TOP17 |
| `zones` | Zone 1/2/Both | TOP94 |
| `heating_mode` | Compensation/Direct | TOP76 |
| `three_way_valve` | Room/DHW/Defrost | TOP20 |

### Numbers (number platform)

| Type | Description | SET# |
|------|-------------|------|
| `dhw_target_temp` | DHW Target (40-75°C) | SET11 |
| `z1_heat_target_temp` | Zone 1 Heat Target | SET5 |
| `z2_heat_target_temp` | Zone 2 Heat Target | SET6 |
| `z1_cool_target_temp` | Zone 1 Cool Target | SET7 |
| `z2_cool_target_temp` | Zone 2 Cool Target | SET8 |
| `dhw_heat_delta` | DHW Heat Delta | SET18 |
| `heat_delta` | Floor Heat Delta | SET19 |
| `cool_delta` | Floor Cool Delta | SET20 |

### Switches (switch platform)

| Topic | Description |
|-------|-------------|
| `force_dhw` | Force DHW Heating |
| `quiet_mode` | Toggle Quiet Mode |

### Text Sensors (text_sensor platform)

| Topic | Description | TOP# |
|-------|-------------|------|
| `error` | Error Code | TOP44 |
| `heat_pump_model` | Heat Pump Model ID | TOP92 |

## Protocol Details

- **Baud Rate:** 9600
- **Parity:** EVEN
- **Stop Bits:** 1
- **Data Packet:** 203 bytes
- **Update Interval:** 30s recommended

## Supported Models

Panasonic Aquarea series with CN-CNT connector:
- J, K, L, M, P generations
- All-in-One and Split models
- Mono-bloc and Bi-bloc configurations

## Troubleshooting

### No Data Received
1. Check TX/RX wiring (may need to swap)
2. Verify level shifter is working
3. Check UART settings (9600, 8E1)

### Invalid Checksums
1. Use shielded cables
2. Add ferrite beads
3. Keep wires away from power cables

### ESP Restarts
1. Verify 5V power supply is stable
2. Check for GPIO conflicts
3. Reduce log level to WARN

## Documentation

- [Climate Control](docs/CLIMATE.md)
- [Water Heater](docs/WATER_HEATER.md)
- [Debugging Guide](docs/DEBUGGING.md)

## Credits

Based on the original [HeishaMon](https://github.com/Egyras/HeishaMon) project by Egyras.

## License

MIT License - See LICENSE file for details.
