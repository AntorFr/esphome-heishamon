import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon", "switch"]
CODEOWNERS = ["@AntorFR"]

HeishamonSwitch = heishamon_ns.class_("HeishamonSwitch", switch.Switch)

# Topics for switches (states mapped from pump data)
HEISHA_SWITCH_TOPICS = {
    # Basic switches (Phase 1)
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

# Topics that can also be controlled: SET command sent on toggle
# (value 1 on ON, 0 on OFF; for quiet_mode ON means level 1).
# Topics not listed here stay read-only (state display only).
HEISHA_SWITCH_COMMANDS = {
    "force_dhw": "SetForceDHW",
    "holiday_mode": "SetHolidayMode",
    "heatpump_state": "SetHeatpump",
    "sterilization": "SetForceSterilization",
    "quiet_mode": "SetQuietMode",
    "main_schedule_state": "SetMainSchedule",
    "alt_external_sensor": "SetAltExternalSensor",
    "external_control": "SetExternalControl",
    "external_error_signal": "SetExternalError",
    "external_compressor_control": "SetExternalCompressorControl",
    "external_heat_cool_control": "SetExternalHeatCoolControl",
    "bivalent_control": "SetBivalentControl",
    "buffer_installed": "SetBuffer",
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
    cg.add(var.set_parent(parent))

    # Controllable topics get their SET command; others stay read-only
    command = HEISHA_SWITCH_COMMANDS.get(config["topic"])
    if command is not None:
        cg.add(var.set_command(command))

    # Register the state callback with the parent
    cg.add(parent.register_switch_callback(
        config["topic"],
        cg.RawExpression(f"[=](bool value) {{ {var}->publish_state(value); }}")
    ))
