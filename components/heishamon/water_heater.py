# DHW (Domestic Hot Water) Climate Component
# Using climate until ESPHome supports water_heater natively
# PR in progress for water_heater support

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import (
    CONF_ID,
    CONF_NAME,
)
from . import HeishamonComponent, CONF_HEISHAMON_ID, heishamon_ns

DEPENDENCIES = ["heishamon", "climate"]

# Water Heater Component (implemented as Climate)
HeishamonWaterHeater = heishamon_ns.class_(
    "HeishamonWaterHeater", climate.Climate, cg.Component
)

# Configuration schema for DHW (Water Heater as Climate)
CONFIG_SCHEMA = climate.climate_schema(HeishamonWaterHeater).extend({
    cv.GenerateID(CONF_HEISHAMON_ID): cv.use_id(HeishamonComponent),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    parent = await cg.get_variable(config[CONF_HEISHAMON_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.register_water_heater(var))
