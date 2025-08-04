import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon"]

HeishamonSwitch = heishamon_ns.class_("HeishamonSwitch", switch.Switch, cg.Component)

# Available commands with their parameters
HEISHA_COMMANDS = {
    "force_dhw": {"name": "Force DHW", "command_type": "boolean"},
    "force_defrost": {"name": "Force defrost", "command_type": "boolean"},
    "holiday_mode": {"name": "Holiday mode", "command_type": "boolean"},
    "heatpump_state": {"name": "Heat pump state", "command_type": "boolean"},
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
