import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon", "switch"]
CODEOWNERS = ["@AntorFR"]

HeishamonSwitch = heishamon_ns.class_("HeishamonSwitch", switch.Switch)

# Topics for switches (read-only states mapped from pump data)
HEISHA_SWITCH_TOPICS = {
    # Basic switches (Phase 1) - These are READ-ONLY states from pump
    "force_dhw": "Force DHW Mode Active",
    "holiday_mode": "Holiday Mode Active", 
    "heatpump_state": "Heat Pump State",
    "sterilization": "Sterilization Active",
    "quiet_mode": "Quiet Mode Active",
    
    # PHASE 2: Advanced switches from HA module (READ-ONLY)
    "buffer_installed": "Buffer Tank Installed",
    "external_control": "External Control Enabled",
    "external_error_signal": "External Error Signal",
    "external_compressor_control": "External Compressor Control",
    "external_heat_cool_control": "External Heat Cool Control",
    "bivalent_control": "Bivalent Control",
    "main_schedule_state": "Main Thermostat Schedule",
    "alt_external_sensor": "Use External Outdoor Sensor",
    
    # GPIO control (for HeishaMon hardware)
    "relay_1": "Relay 1 State",
    "relay_2": "Relay 2 State",
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HeishamonSwitch),
        cv.GenerateID("heishamon_id"): cv.use_id(HeishamonComponent),
        cv.Required("topic"): cv.one_of(*HEISHA_SWITCH_TOPICS.keys(), lower=True),
    }
).extend(switch.switch_schema(HeishamonSwitch))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await switch.register_switch(var, config)

    parent = await cg.get_variable(config["heishamon_id"])
    
    # Register the callback with the parent (read-only switches)
    cg.add(parent.register_switch_callback(
        config["topic"],
        cg.RawExpression(f"[=](bool value) {{ {var}->publish_state(value); }}")
    ))
