import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon"]

HeishamonBinarySensor = heishamon_ns.class_("HeishamonBinarySensor", binary_sensor.BinarySensor)

# Topics for binary sensors
HEISHA_BINARY_TOPICS = {
    # Basic binary sensors (Phase 1)
    "heatpump_state": "Heat pump state",
    "defrosting_state": "Defrosting state", 
    "dhw_heater_state": "DHW heater state",
    "room_heater_state": "Room heater state",
    "sterilization_state": "Sterilization state",
    
    # PHASE 2: Advanced binary sensors from HA module
    "quiet_mode_schedule": "Quiet Mode Schedule",
    "internal_heater_state": "Internal Heater State",
    "external_heater_state": "External Heater State", 
    "force_heater_state": "Force Heater Status",
    "pump_running": "Pump Running",
    "dhw_installed": "DHW Installed",
    "anti_freeze_mode": "Anti Freeze Mode",
    "optional_pcb": "Optional PCB Enabled",
    "bivalent_advanced_heat": "Bivalent Advanced Heat",
    "bivalent_advanced_dhw": "Bivalent Advanced DHW",
    
    # Optional PCB binary sensors
    "z1_water_pump": "Zone 1 Water Pump Request",
    "z2_water_pump": "Zone 2 Water Pump Request",
    "pool_water_pump": "Pool Water Pump Request",
    "solar_water_pump": "Solar Water Pump Request",
    "alarm_state": "Alarm State",
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
    await binary_sensor.register_binary_sensor(var, config)

    parent = await cg.get_variable(config["heishamon_id"])
    
    # Register the callback with the parent
    cg.add(parent.register_binary_sensor_callback(
        config["topic"],
        cg.RawExpression(f"[=](bool value) {{ {var}->publish_state(value); }}")
    ))
