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

### Temperature Ranges

The temperature ranges depend on the operation mode:

- **Heating Mode**: 20°C to 55°C (water temperature control)
- **Cooling Mode**: 5°C to 25°C (water temperature control)
- **Step**: 1°C

### Operation Modes

- **Off**: Zone is disabled
- **Heat**: Zone heating enabled (requires `supports_heat: true`)
- **Cool**: Zone cooling enabled (requires `supports_cool: true`)

### Zone Coordination

The climate component automatically coordinates with the heat pump's global operating modes:

1. **Zone Control**: When you turn on a zone, it enables that specific zone
2. **Global Mode**: When you enable heating/cooling, it enables the global heat pump mode
3. **Auto Disable**: When all zones are turned off, the global mode may be disabled

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

- Confirm that the heat pump supports the requested operation mode
- Check that global heat pump modes are enabled
- Verify that zone control is available on your heat pump model

### Temperature Range Issues

- The component uses water temperature control, not room temperature
- Ranges are based on typical Panasonic Aquarea specifications
- Some heat pump models may have different ranges

## Related Components

- [HeishaMon Core Component](../README.md)
- [HeishaMon Sensors](SENSORS.md)
- [HeishaMon Switches](SWITCHES.md)

## Note

This climate component is designed specifically for Panasonic Aquarea heat pumps compatible with HeishaMon. The behavior may vary depending on your specific heat pump model and configuration.
