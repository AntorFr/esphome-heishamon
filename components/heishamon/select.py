import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon"]

HeishamonSelect = heishamon_ns.class_("HeishamonSelect", select.Select, cg.Component)

# Available select entities based on HA module analysis
HEISHA_SELECT_OPTIONS = {
    "bivalent_mode": {
        "name": "Bivalent Mode",
        "command": "SetBivalentMode",
        "options": ["Disabled", "Alternative", "Hybrid"],
        "default": "Disabled"
    },
    "external_pad_heater_type": {
        "name": "External Pad Heater Type", 
        "command": "SetExternalPadHeater",
        "options": ["Disabled", "Type 1", "Type 2", "Type 3"],
        "default": "Disabled"
    },
    "smart_grid_mode": {
        "name": "Smart Grid Mode",
        "command": "SetSmartGridMode", 
        "options": ["Disabled", "SG Ready", "Power Control", "Normal"],
        "default": "Disabled"
    },
    "heating_mode": {
        "name": "Heating Mode",
        "command": "SetHeatingMode",
        "options": ["Compensated", "Direct", "Water", "Room"],
        "default": "Compensated"
    },
    "cooling_mode": {
        "name": "Cooling Mode", 
        "command": "SetCoolingMode",
        "options": ["Compensated", "Direct", "Water", "Room"],
        "default": "Compensated"
    }
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HeishamonSelect),
        cv.GenerateID("heishamon_id"): cv.use_id(HeishamonComponent),
        cv.Required("select_type"): cv.one_of(*HEISHA_SELECT_OPTIONS.keys(), lower=True),
    }
).extend(select.select_schema(HeishamonSelect))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await select.register_select(var, config)

    parent = await cg.get_variable(config["heishamon_id"])
    cg.add(var.set_parent(parent))
    cg.add(var.set_select_type(config["select_type"]))
    
    # Configure options based on select type
    select_config = HEISHA_SELECT_OPTIONS[config["select_type"]]
    cg.add(var.set_command(select_config["command"]))
    cg.add(var.set_options(select_config["options"]))
    cg.add(var.set_initial_option(select_config["default"]))
