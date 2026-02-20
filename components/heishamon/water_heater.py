# DHW (Domestic Hot Water) using native ESPHome WaterHeater platform

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import water_heater
from esphome.const import (
    CONF_ID,
)
from . import HeishamonComponent, CONF_HEISHAMON_ID, heishamon_ns

DEPENDENCIES = ["heishamon", "water_heater"]

# Water Heater Component (native ESPHome WaterHeater platform)
HeishamonWaterHeater = heishamon_ns.class_(
    "HeishamonWaterHeater", water_heater.WaterHeater, cg.Component
)

# Configuration schema for DHW Water Heater
CONFIG_SCHEMA = water_heater.water_heater_schema(HeishamonWaterHeater).extend({
    cv.GenerateID(CONF_HEISHAMON_ID): cv.use_id(HeishamonComponent),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await water_heater.new_water_heater(config)
    await cg.register_component(var, config)
    parent = await cg.get_variable(config[CONF_HEISHAMON_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_water_heater(var))

