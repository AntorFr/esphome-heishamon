# Phase 1 - Advanced Sensors Documentation

## 🎯 Overview

Phase 1 adds critical sensors that were missing compared to the Home Assistant module, focusing on the most useful monitoring capabilities for heat pump users.

## 📊 New Sensors Added

### Power & Energy Monitoring

#### Separated Power Sensors
Instead of just generic "heat power", we now have function-specific power monitoring:

| Sensor | Topic | Description | Unit |
|--------|--------|-------------|------|
| `dhw_power_production` | DHW Power Production | Power produced for domestic hot water | W |
| `dhw_power_consumption` | DHW Power Consumption | Power consumed for domestic hot water | W |
| `cool_power_production` | Cool Power Production | Power produced for cooling | W |
| `cool_power_consumption` | Cool Power Consumption | Power consumed for cooling | W |

These allow you to:
- Monitor DHW heating efficiency separately
- Track cooling power usage (if your unit supports cooling)
- Calculate separate COP for different functions

#### COP (Coefficient of Performance)
| Sensor | Topic | Description | Unit |
|--------|--------|-------------|------|
| `cop` | Coefficient of Performance | Heat production / Heat consumption ratio | - |

**Why COP is important:**
- Measures heat pump efficiency (higher = better)
- Typical values: 2.5-4.5 (means 1kW electricity produces 2.5-4.5kW heat)
- Helps optimize operation and detect issues
- COP < 2.0 may indicate problems
- COP > 6.0 is probably a measurement error

### Zone Monitoring

| Sensor | Topic | Description | Unit |
|--------|--------|-------------|------|
| `z1_water_temp` | Zone 1 Water Temperature | Water temperature in zone 1 circuit | °C |
| `z2_water_temp` | Zone 2 Water Temperature | Water temperature in zone 2 circuit | °C |
| `room_thermostat_temp` | Room Thermostat Temperature | Temperature from room thermostat | °C |
| `z1_valve_pid` | Zone 1 Valve PID | Zone 1 mixing valve control percentage | % |
| `z2_valve_pid` | Zone 2 Valve PID | Zone 2 mixing valve control percentage | % |

**Usage:**
- Monitor individual zone performance
- Detect imbalanced heating/cooling
- Optimize zone control settings

### Temperature Deltas

| Sensor | Topic | Description | Unit |
|--------|--------|-------------|------|
| `heat_delta` | Heat Delta Temperature | Temperature difference for heating circuit | °C |
| `cool_delta` | Cool Delta Temperature | Temperature difference for cooling circuit | °C |

**Purpose:**
- Monitor system efficiency
- Detect flow problems
- Optimize heating curves

### Maintenance Monitoring

| Sensor | Topic | Description | Unit |
|--------|--------|-------------|------|
| `compressor_operating_hours` | Compressor Operating Hours | Total compressor runtime | h |
| `room_heater_operating_hours` | Room Heater Operating Hours | Electric heater runtime for room heating | h |
| `dhw_heater_operating_hours` | DHW Heater Operating Hours | Electric heater runtime for DHW | h |

**Benefits:**
- Plan maintenance schedules
- Monitor component wear
- Detect excessive backup heater usage

### Holiday & Comfort Settings

| Sensor | Topic | Description | Unit |
|--------|--------|-------------|------|
| `room_holiday_shift_temp` | Room Holiday Shift Temperature | Temperature offset during holiday mode | °C |
| `dhw_holiday_shift_temp` | DHW Holiday Shift Temperature | DHW temperature offset during holiday mode | °C |
| `buffer_temp` | Buffer Temperature | Buffer tank temperature (if installed) | °C |

## 🏠 Configuration Examples

### ESP32 (Full Features)
```yaml
sensor:
  # Efficiency monitoring
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "cop"
    name: "Heat Pump COP"
    accuracy_decimals: 2
    
  # Zone monitoring
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "z1_water_temp"
    name: "Zone 1 Water Temperature"
    
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "z1_valve_pid"
    name: "Zone 1 Valve Control"
    
  # Maintenance tracking
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "compressor_operating_hours"
    name: "Compressor Hours"
    
  # Power monitoring
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "dhw_power_consumption"
    name: "DHW Power Usage"
```

### ESP8266 (Essential Only)
```yaml
sensor:
  # Most important for monitoring
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "cop"
    name: "Heat Pump COP"
    accuracy_decimals: 2
    
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "z1_water_temp"
    name: "Zone 1 Temperature"
    
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "compressor_operating_hours"
    name: "Compressor Hours"
```

## 📈 Home Assistant Integration

### Creating Utility Meters
Track daily/monthly energy consumption:

```yaml
# configuration.yaml
utility_meter:
  daily_heat_energy:
    source: sensor.heat_power_consumption
    cycle: daily
    
  monthly_dhw_energy:
    source: sensor.dhw_power_consumption
    cycle: monthly
```

### COP Monitoring Automation
```yaml
# automation.yaml
- id: low_cop_alert
  alias: "Low COP Alert"
  trigger:
    platform: numeric_state
    entity_id: sensor.heat_pump_cop
    below: 2.0
    for: "00:15:00"
  action:
    - service: notify.mobile_app
      data:
        message: "Heat pump COP is low ({{ trigger.to_state.state }}). Check system."
```

### Zone Temperature Monitoring
```yaml
- id: zone_temperature_imbalance
  alias: "Zone Temperature Imbalance"
  trigger:
    platform: template
    value_template: >
      {{ (states('sensor.zone_1_water_temperature')|float - 
          states('sensor.zone_2_water_temperature')|float)|abs > 5 }}
  action:
    - service: notify.mobile_app
      data:
        message: "Zone temperature imbalance detected. Check mixing valves."
```

## 🔧 Troubleshooting

### COP Shows -1 or No Value
- Check that both heat production and consumption sensors are working
- Ensure heat pump is actually running (COP only calculated during operation)
- Verify power consumption > 100W (minimum threshold for calculation)

### Zone Temperatures Not Available
- Check if your heat pump has multi-zone support
- Verify optional PCB is enabled if needed: `optional_pcb: true`
- Some sensors may not be available on all heat pump models

### Operating Hours Not Updating
- Operating hours update slowly (hourly increments)
- Check that heat pump communication is stable
- Verify sensor byte indices match your heat pump model

## 🚀 Next Steps

Phase 1 provides the foundation for advanced monitoring. Next phases will add:
- **Phase 2**: Select entities for mode control
- **Phase 3**: Advanced climate controls and number entities
- **Phase 4**: Water heater component (when ESPHome supports it)

## 📚 Technical Details

### Data Source Mapping
Sensors map to specific bytes in the HeishaMon protocol:

| Topic | Byte Index | Decode Function | Notes |
|-------|------------|-----------------|-------|
| `cop` | Calculated | get_cop() | Uses bytes 193+194 |
| `z1_water_temp` | 36 | get_int_minus_128() | Standard temp conversion |
| `compressor_operating_hours` | 88 | get_operating_hours() | Direct value |
| `heat_delta` | 23 | get_heat_delta() | 0.5°C resolution |

### COP Calculation
```cpp
float cop = heat_production / heat_consumption;
// Valid range: 1.0 to 8.0
// Returns -1 if invalid or not running
```

This ensures reliable COP measurements during actual heat pump operation.
