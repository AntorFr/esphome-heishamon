
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import water_heater
from esphome.const import (
    CONF_ID,
    CONF_NAME,
)
from . import HeishamonComponent, CONF_HEISHAMON_ID, heishamon_ns

DEPENDENCIES = ["heishamon"]

# Water Heater Component
HeishamonWaterHeater = heishamon_ns.class_(
    "HeishamonWaterHeater", water_heater.WaterHeater, cg.Component
)

# Configuration schema for Water Heater
CONFIG_SCHEMA = water_heater.water_heater_schema(HeishamonWaterHeater).extend({
    cv.GenerateID(CONF_HEISHAMON_ID): cv.use_id(HeishamonComponent),
    cv.Optional(CONF_NAME, default="DHW"): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await water_heater.register_water_heater(var, config)
    parent = await cg.get_variable(config[CONF_HEISHAMON_ID])
    cg.add(parent.register_water_heater(var))
