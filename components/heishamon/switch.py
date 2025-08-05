import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon"]

HeishamonSwitch = heishamon_ns.class_("HeishamonSwitch", switch.Switch, cg.Component)

# Available commands with their parameters
HEISHA_COMMANDS = {
    # Basic commands (Phase 1)
    "force_dhw": {"name": "Force DHW", "command_type": "boolean"},
    "force_defrost": {"name": "Force defrost", "command_type": "boolean"},
    "holiday_mode": {"name": "Holiday mode", "command_type": "boolean"},
    "heatpump_state": {"name": "Heat pump state", "command_type": "boolean"},
    
    # PHASE 2: Advanced switches from HA module analysis
    "buffer_installed": {"name": "Buffer Tank Installed", "command_type": "boolean"},
    "external_control": {"name": "External Control Enabled", "command_type": "boolean"},
    "external_error_signal": {"name": "External Error Signal", "command_type": "boolean"},
    "external_compressor_control": {"name": "External Compressor Control", "command_type": "boolean"},
    "external_heat_cool_control": {"name": "External Heat Cool Control", "command_type": "boolean"},
    "bivalent_control": {"name": "Bivalent Control", "command_type": "boolean"},
    "main_schedule_state": {"name": "Main Thermostat Schedule", "command_type": "boolean"},
    "alt_external_sensor": {"name": "Use External Outdoor Sensor", "command_type": "boolean"},
    "sterilization": {"name": "Force Sterilization", "command_type": "boolean"},
    "quiet_mode": {"name": "Quiet Mode", "command_type": "boolean"},
    
    # GPIO control (for HeishaMon hardware)
    "relay_1": {"name": "Relay 1", "command_type": "boolean"},
    "relay_2": {"name": "Relay 2", "command_type": "boolean"},
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HeishamonSwitch),
        cv.GenerateID("heishamon_id"): cv.use_id(HeishamonComponent),
        cv.Required("command"): cv.one_of(*HEISHA_COMMANDS.keys(), lower=True),
    }
).extend(switch.switch_schema(HeishamonSwitch))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)

    parent = await cg.get_variable(config["heishamon_id"])
    cg.add(var.set_parent(parent))
    cg.add(var.set_command(config["command"]))
