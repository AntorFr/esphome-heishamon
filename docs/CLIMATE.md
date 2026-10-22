# Climate (Thermostat) Component

The HeishaMon climate component provides thermostat functionality for Panasonic Aquarea heat pumps, allowing you to control heating and cooling zones directly from ESPHome.

## Overview

This component creates climate entities (thermostats) that can:
- Control zone heating and cooling independently
- Set target temperatures for each zone
- Monitor current temperatures
- Handle multiple operation modes (Heat, Cool, Off)
- Coordinate with the heat pump's global operation modes

## Configuration

### Basic Configuration

```yaml
climate:
  # Zone 1 Heating Thermostat
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Zone 1 Heating"
    zone_id: 1
    supports_heat: true
    supports_cool: false
    
  # Zone 2 Heating Thermostat  
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Zone 2 Heating"
    zone_id: 2
    supports_heat: true
    supports_cool: false
```

### Advanced Configuration with Cooling

```yaml
climate:
  # Zone 1 - Separate heating and cooling thermostats
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Zone 1 Heating"
    zone_id: 1
    supports_heat: true
    supports_cool: false
    
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Zone 1 Cooling"
    zone_id: 1
    supports_heat: false
    supports_cool: true
    
  # Zone 2 - Separate heating and cooling thermostats
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Zone 2 Heating"
    zone_id: 2
    supports_heat: true
    supports_cool: false
    
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Zone 2 Cooling"
    zone_id: 2
    supports_heat: false
    supports_cool: true
```

## Configuration Variables

- **heishamon_id** (**Required**, [ID](https://esphome.io/guides/configuration-types.html#config-id)): The ID of the HeishaMon component.
- **zone_id** (**Required**, int): The zone number (1 or 2).
- **supports_heat** (*Optional*, boolean): Whether this thermostat controls heating. Defaults to `true`.
- **supports_cool** (*Optional*, boolean): Whether this thermostat controls cooling. Defaults to `false`.
- **name** (*Optional*, string): The name of the climate entity.
- All other options from [Climate](https://esphome.io/components/climate/index.html).

## Behavior

### Target Temperature

What the target means depends on how the zone is set up on the heat pump:

| Zone sensor (TOP111/112) | Heating/cooling mode (TOP76/81) | Target shown and set |
|---|---|---|
| Water temperature, External thermostat | Compensation curve | Water temperature = curve value + shift |
| Water temperature, External thermostat | Direct | Water temperature (absolute) |
| Internal thermostat, Thermistor | any | Room temperature (absolute) |

On the compensation curve the heat pump only accepts a **shift of -5 to +5 K**
(the raw TOP27 value). The thermostat therefore computes the curve's water
temperature for the current outdoor temperature (TOP14 and the curve settings
TOP29-32), displays `curve + shift`, and turns a new target into the matching
shift. A target beyond ±5 K of the curve is clamped, with a warning in the log.
The displayed value moves with the outdoor temperature, as the curve does.

Visual ranges: 20-65 °C (water, heating), 5-25 °C (water, cooling),
10-30 °C (room), step 1 °C.

### Operation Modes

- **Heat**: switches the operation mode to *Heat* (or *Heat+DHW* when hot water
  was enabled), adds the zone to the active zones and powers the heat pump on if needed
- **Cool**: same with *Cool* / *Cool+DHW* (requires `supports_cool: true`)
- **Off**: if the other zone is active, only this zone is removed; otherwise the
  heat pump goes to *DHW only* when hot water was enabled, or is powered off

The displayed mode always reflects the heat pump state (power, operation mode,
active zones); *Auto* modes show as Heat or Cool depending on the active side.

### Integration with Home Assistant

The climate entities will appear in Home Assistant as standard thermostats with:

- Current temperature display
- Target temperature control
- Mode selection (Off, Heat, Cool)
- Standard thermostat card interface

## Examples

### Simple Single Zone Setup

```yaml
heishamon:
  id: heisha_main
  # ... other heishamon config

climate:
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Main Zone Heating"
    zone_id: 1
    supports_heat: true
```

### Dual Zone with Heating and Cooling

```yaml
heishamon:
  id: heisha_main
  # ... other heishamon config

climate:
  # Zone 1
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Living Room Heating"
    zone_id: 1
    supports_heat: true
    
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Living Room Cooling"
    zone_id: 1
    supports_cool: true
    
  # Zone 2  
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Bedroom Heating"
    zone_id: 2
    supports_heat: true
    
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Bedroom Cooling"
    zone_id: 2
    supports_cool: true
```

### With Home Assistant Automations

You can create automations in Home Assistant to control the thermostats:

```yaml
# Home Assistant automation example
automation:
  - alias: "Morning Heating"
    trigger:
      platform: time
      at: "06:00:00"
    action:
      - service: climate.set_temperature
        entity_id: climate.living_room_heating
        data:
          temperature: 22
          hvac_mode: heat
```

## Troubleshooting

### No Temperature Updates

- Ensure the HeishaMon component is receiving data from the heat pump
- Check that zone temperature sensors are configured in the HeishaMon topics
- Verify UART connection and communication

### Mode Changes Not Working

- Mode commands require active mode (`listen_only: false`)
- Check the log for `Zone N demand` lines and the following heat pump frame

### Target Is Empty

- On the compensation curve the target needs the curve settings and the outdoor
  temperature; it appears with the first full heat pump frame

## Related Components

- [HeishaMon Core Component](../README.md)
- [HeishaMon Sensors](SENSORS.md)
- [HeishaMon Switches](SWITCHES.md)

## Note

This climate component is designed specifically for Panasonic Aquarea heat pumps compatible with HeishaMon. The behavior may vary depending on your specific heat pump model and configuration.
