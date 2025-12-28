import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon", "select"]
CODEOWNERS = ["@AntorFR"]

HeishamonSelect = heishamon_ns.class_("HeishamonSelect", select.Select, cg.Component)

# Available select entities based on HeishaMon protocol analysis
# Reference: https://github.com/Egyras/HeishaMon and heishamon-homeassistant
HEISHA_SELECT_OPTIONS = {
    # ============ CONTROL SELECTS (can send commands) ============
    
    # SET3 - Quiet Mode (0-4, where 4=scheduled)
    "quiet_mode": {
        "name": "Quiet Mode",
        "command": "SetQuietMode",
        "topic": "quiet_mode_level",  # Also listen to current state
        "options": ["Off", "Level 1", "Level 2", "Level 3", "Scheduled"],
        "default": "Off",
        "icon": "mdi:volume-off"
    },
    
    # SET4 - Powerful Mode (0-3 = 0/30/60/90 minutes)
    "powerful_mode": {
        "name": "Powerful Mode",
        "command": "SetPowerfulMode",
        "topic": "powerful_mode_time",
        "options": ["Off", "30 min", "60 min", "90 min"],
        "default": "Off",
        "icon": "mdi:rocket-launch"
    },
    
    # SET9 - Operating Mode (0-8)
    "operating_mode": {
        "name": "Operating Mode",
        "command": "SetOperationMode",
        "topic": "operation_mode",
        "options": [
            "Heat only",        # 0
            "Cool only",        # 1
            "Auto(Heat)",       # 2
            "DHW only",         # 3
            "Heat+DHW",         # 4
            "Cool+DHW",         # 5
            "Auto(Heat)+DHW",   # 6
            "Auto(Cool)",       # 7
            "Auto(Cool)+DHW"    # 8
        ],
        "default": "Heat+DHW",
        "icon": "mdi:heat-pump"
    },
    
    # SET17 - Active Zones (0-2)
    "zones": {
        "name": "Active Zones",
        "command": "SetZones",
        "topic": "zones_state",
        "options": ["Zone 1 only", "Zone 2 only", "Zone 1 + Zone 2"],
        "default": "Zone 1 only",
        "icon": "mdi:home-thermometer"
    },
    
    # SET35 - Bivalent Mode (0-2)
    "bivalent_mode": {
        "name": "Bivalent Mode",
        "command": "SetBivalentMode",
        "topic": "bivalent_mode",
        "options": ["Alternative", "Parallel", "Advanced Parallel"],
        "default": "Alternative",
        "icon": "mdi:heat-wave"
    },
    
    # SET26 - External Pad Heater Type (0-2)
    "external_pad_heater": {
        "name": "External Pad Heater",
        "command": "SetExternalPadHeater",
        "topic": "external_pad_heater",
        "options": ["Disabled", "Type-A", "Type-B"],
        "default": "Disabled",
        "icon": "mdi:radiator"
    },
    
    # Smart Grid Mode (Optional PCB command)
    "smart_grid_mode": {
        "name": "Smart Grid Mode",
        "command": "SetSmartGridMode", 
        "topic": "smart_grid_mode",
        "options": ["Normal", "Capacity 1", "HP/DHW Off", "Capacity 2"],
        "default": "Normal",
        "icon": "mdi:transmission-tower"
    },
    
    # ============ READ-ONLY SELECTS (topic-based, no command) ============
    
    # Heating Mode - Read from TOP76
    "heating_mode": {
        "name": "Heating Mode",
        "topic": "heating_mode",
        "options": ["Compensation curve", "Direct"],
        "default": "Compensation curve",
        "icon": "mdi:thermometer-lines"
    },
    
    # Cooling Mode - Read from TOP81
    "cooling_mode": {
        "name": "Cooling Mode", 
        "topic": "cooling_mode",
        "options": ["Compensation curve", "Direct"],
        "default": "Compensation curve",
        "icon": "mdi:snowflake-thermometer"
    },
    
    # 3-Way Valve State - Read from TOP20
    "three_way_valve": {
        "name": "3-Way Valve State",
        "topic": "three_way_valve_state",
        "options": ["Room", "DHW"],
        "default": "Room",
        "icon": "mdi:valve"
    },
    
    # Holiday Mode State - Read from TOP19
    "holiday_mode_state": {
        "name": "Holiday Mode State",
        "topic": "holiday_mode_state",
        "options": ["Off", "Scheduled", "Active"],
        "default": "Off",
        "icon": "mdi:beach"
    },
    
    # Solar Mode - Read from TOP101
    "solar_mode": {
        "name": "Solar Mode",
        "topic": "solar_mode",
        "options": ["Disabled", "Buffer", "DHW"],
        "default": "Disabled",
        "icon": "mdi:solar-power"
    },
    
    # Pump Flowrate Mode - Read from TOP106 (J-series)
    "pump_flowrate_mode": {
        "name": "Pump Flowrate Mode",
        "topic": "pump_flowrate_mode",
        "options": ["DeltaT", "Max flow"],
        "default": "DeltaT",
        "icon": "mdi:pump"
    },
    
    # Liquid Type - Read from TOP107
    "liquid_type": {
        "name": "Liquid Type",
        "topic": "liquid_type",
        "options": ["Water", "Glycol"],
        "default": "Water",
        "icon": "mdi:water"
    },
    
    # Zone 1 Sensor Settings - Read from TOP112
    "z1_sensor_settings": {
        "name": "Zone 1 Sensor",
        "topic": "z1_sensor_settings",
        "options": ["Water temperature", "External thermostat", "Internal thermostat", "Thermistor"],
        "default": "Water temperature",
        "icon": "mdi:thermometer"
    },
    
    # Zone 2 Sensor Settings - Read from TOP111
    "z2_sensor_settings": {
        "name": "Zone 2 Sensor",
        "topic": "z2_sensor_settings",
        "options": ["Water temperature", "External thermostat", "Internal thermostat", "Thermistor"],
        "default": "Water temperature",
        "icon": "mdi:thermometer"
    },
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HeishamonSelect),
        cv.GenerateID("heishamon_id"): cv.use_id(HeishamonComponent),
        cv.Required("type"): cv.one_of(*HEISHA_SELECT_OPTIONS.keys(), lower=True),
    }
).extend(select.select_schema(HeishamonSelect))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Get options for this select type
    select_config = HEISHA_SELECT_OPTIONS[config["type"]]
    options = select_config["options"]
    
    # register_select handles options via traits
    await select.register_select(var, config, options=options)

    parent = await cg.get_variable(config["heishamon_id"])
    cg.add(var.set_parent(parent))
    cg.add(var.set_select_type(config["type"]))
    
    # Configure mode based on select type
    cg.add(var.set_initial_option(select_config["default"]))
    
    # Check if this has a command (control mode) or just topic (read-only)
    if "command" in select_config:
        # Command-based select for control mode
        cg.add(var.set_command(select_config["command"]))
        cg.add(var.set_listen_only(False))
        # Also set topic if available for reading current state
        if "topic" in select_config:
            cg.add(var.set_topic(select_config["topic"]))
    else:
        # Topic-based select for listen-only mode (no command)
        cg.add(var.set_topic(select_config["topic"]))
        cg.add(var.set_listen_only(True))
