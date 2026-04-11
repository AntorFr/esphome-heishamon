import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, DEVICE_CLASS_RUNNING, DEVICE_CLASS_PROBLEM
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon"]

HeishamonBinarySensor = heishamon_ns.class_("HeishamonBinarySensor", binary_sensor.BinarySensor)

# Complete binary sensor topics based on HeishaMon protocol
# Reference: https://github.com/Egyras/HeishaMon
HEISHA_BINARY_TOPICS = {
    # ============== MAIN STATE SENSORS ==============
    
    # TOP0 - Heat Pump State
    "heatpump_state": {
        "name": "Heat Pump State",
        "topic_id": 0,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:heat-pump"
    },
    
    # TOP2 - Force DHW State
    "force_dhw_state": {
        "name": "Force DHW State",
        "topic_id": 2,
        "device_class": None,
        "icon": "mdi:water-boiler"
    },
    
    # TOP3 - Quiet Mode Schedule Active
    "quiet_mode_schedule": {
        "name": "Quiet Mode Schedule",
        "topic_id": 3,
        "device_class": None,
        "icon": "mdi:calendar-clock"
    },
    
    # TOP13 - Main Schedule State
    "main_schedule_state": {
        "name": "Main Schedule State",
        "topic_id": 13,
        "device_class": None,
        "icon": "mdi:calendar"
    },
    
    # TOP26 - Defrosting State
    "defrosting_state": {
        "name": "Defrosting State",
        "topic_id": 26,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:snowflake-melt"
    },
    
    # ============== HEATER STATES ==============
    
    # TOP58 - DHW Heater State (Blocked/Free)
    "dhw_heater_state": {
        "name": "DHW Heater State",
        "topic_id": 58,
        "device_class": None,
        "icon": "mdi:water-boiler"
    },
    
    # TOP59 - Room Heater State (Blocked/Free)
    "room_heater_state": {
        "name": "Room Heater State",
        "topic_id": 59,
        "device_class": None,
        "icon": "mdi:radiator"
    },
    
    # TOP60 - Internal Heater State
    "internal_heater_state": {
        "name": "Internal Heater State",
        "topic_id": 60,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:heating-coil"
    },
    
    # TOP61 - External Heater State
    "external_heater_state": {
        "name": "External Heater State",
        "topic_id": 61,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:radiator"
    },
    
    # TOP68 - Force Heater State
    "force_heater_state": {
        "name": "Force Heater State",
        "topic_id": 68,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:radiator"
    },
    
    # TOP69 - Sterilization State
    "sterilization_state": {
        "name": "Sterilization State",
        "topic_id": 69,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:bacteria-outline"
    },
    
    # ============== INSTALLATION STATES ==============
    
    # TOP99 - Buffer Installed
    "buffer_installed": {
        "name": "Buffer Installed",
        "topic_id": 99,
        "device_class": None,
        "icon": "mdi:storage-tank"
    },
    
    # TOP100 - DHW Installed
    "dhw_installed": {
        "name": "DHW Installed",
        "topic_id": 100,
        "device_class": None,
        "icon": "mdi:water-boiler"
    },
    
    # TOP108 - Alternative External Sensor
    "alt_external_sensor": {
        "name": "Alternative External Sensor",
        "topic_id": 108,
        "device_class": None,
        "icon": "mdi:thermometer"
    },
    
    # TOP109 - Anti Freeze Mode
    "anti_freeze_mode": {
        "name": "Anti Freeze Mode",
        "topic_id": 109,
        "device_class": None,
        "icon": "mdi:snowflake-alert"
    },
    
    # TOP110 - Optional PCB Enabled
    "optional_pcb": {
        "name": "Optional PCB Enabled",
        "topic_id": 110,
        "device_class": None,
        "icon": "mdi:expansion-card"
    },
    
    # ============== EXTERNAL CONTROL STATES ==============
    
    # TOP119 - External Control
    "external_control": {
        "name": "External Control",
        "topic_id": 119,
        "device_class": None,
        "icon": "mdi:remote"
    },
    
    # TOP120 - External Heat/Cool Control
    "external_heat_cool_control": {
        "name": "External Heat Cool Control",
        "topic_id": 120,
        "device_class": None,
        "icon": "mdi:remote"
    },
    
    # TOP121 - External Error Signal
    "external_error_signal": {
        "name": "External Error Signal",
        "topic_id": 121,
        "device_class": DEVICE_CLASS_PROBLEM,
        "icon": "mdi:alert"
    },
    
    # TOP122 - External Compressor Control
    "external_compressor_control": {
        "name": "External Compressor Control",
        "topic_id": 122,
        "device_class": None,
        "icon": "mdi:remote"
    },
    
    # ============== PUMP & VALVE STATES ==============
    
    # TOP123 - Zone 2 Pump State
    "z2_pump_state": {
        "name": "Zone 2 Pump State",
        "topic_id": 123,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:pump"
    },
    
    # TOP124 - Zone 1 Pump State
    "z1_pump_state": {
        "name": "Zone 1 Pump State",
        "topic_id": 124,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:pump"
    },
    
    # Derived: Pump Running (from pump speed > 0)
    "pump_running": {
        "name": "Pump Running",
        "topic_id": None,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:pump"
    },
    
    # ============== BIVALENT STATES ==============
    
    # TOP129 - Bivalent Control
    "bivalent_control": {
        "name": "Bivalent Control",
        "topic_id": 129,
        "device_class": None,
        "icon": "mdi:heat-wave"
    },
    
    # TOP132 - Bivalent Advanced Heat
    "bivalent_advanced_heat": {
        "name": "Bivalent Advanced Heat",
        "topic_id": 132,
        "device_class": None,
        "icon": "mdi:heat-wave"
    },
    
    # TOP133 - Bivalent Advanced DHW
    "bivalent_advanced_dhw": {
        "name": "Bivalent Advanced DHW",
        "topic_id": 133,
        "device_class": None,
        "icon": "mdi:water-boiler"
    },
    
    # ============== OPTIONAL PCB SENSORS (OPT0-OPT6) ==============
    
    # OPT0 - Zone 1 Water Pump
    "z1_water_pump": {
        "name": "Zone 1 Water Pump Request",
        "topic_id": "OPT0",
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:pump"
    },
    
    # OPT2 - Zone 2 Water Pump
    "z2_water_pump": {
        "name": "Zone 2 Water Pump Request",
        "topic_id": "OPT2",
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:pump"
    },
    
    # OPT4 - Pool Water Pump
    "pool_water_pump": {
        "name": "Pool Water Pump Request",
        "topic_id": "OPT4",
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:pump"
    },
    
    # OPT5 - Solar Water Pump
    "solar_water_pump": {
        "name": "Solar Water Pump Request",
        "topic_id": "OPT5",
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:pump"
    },
    
    # OPT6 - Alarm State
    "alarm_state": {
        "name": "Alarm State",
        "topic_id": "OPT6",
        "device_class": DEVICE_CLASS_PROBLEM,
        "icon": "mdi:alarm-light"
    },
    
    # ============== LEGACY (for backward compatibility) ==============
    
    "dhw_heating": {
        "name": "DHW Heating Active",
        "topic_id": None,
        "device_class": DEVICE_CLASS_RUNNING,
        "icon": "mdi:water-boiler"
    },
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HeishamonBinarySensor),
        cv.GenerateID("heishamon_id"): cv.use_id(HeishamonComponent),
        cv.Required("topic"): cv.one_of(*HEISHA_BINARY_TOPICS.keys(), lower=True),
    }
).extend(binary_sensor.binary_sensor_schema())

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    
    # Get topic configuration
    topic_config = HEISHA_BINARY_TOPICS[config["topic"]]
    
    # Merge defaults into config for registration
    merged_config = dict(config)
    if topic_config.get("device_class") and "device_class" not in merged_config:
        merged_config["device_class"] = topic_config["device_class"]
    if topic_config.get("icon") and "icon" not in merged_config:
        merged_config["icon"] = topic_config["icon"]
    
    await binary_sensor.register_binary_sensor(var, merged_config)

    parent = await cg.get_variable(config["heishamon_id"])
    
    # Register the callback with the parent
    cg.add(parent.register_binary_sensor_callback(
        config["topic"],
        cg.RawExpression(f"[=](bool value) {{ {var}->publish_state(value); }}")
    ))
