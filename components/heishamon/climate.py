import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import (
    CONF_ID,
)

from . import CONF_HEISHAMON_ID, heishamon_ns, HeishamonComponent

DEPENDENCIES = ["heishamon", "climate"]

CONF_ZONE_ID = "zone_id"
CONF_SUPPORTS_HEAT = "supports_heat"
CONF_SUPPORTS_COOL = "supports_cool"

HeishaMonClimate = heishamon_ns.class_(
    "HeishaMonClimate", climate.Climate, cg.Component
)

CONFIG_SCHEMA = climate.climate_schema(HeishaMonClimate).extend({
    cv.GenerateID(CONF_HEISHAMON_ID): cv.use_id(HeishamonComponent),
    cv.Required(CONF_ZONE_ID): cv.int_range(min=1, max=2),
    cv.Optional(CONF_SUPPORTS_HEAT, default=True): cv.boolean,
    cv.Optional(CONF_SUPPORTS_COOL, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    parent = await cg.get_variable(config[CONF_HEISHAMON_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_zone_id(config[CONF_ZONE_ID]))
    cg.add(var.set_supports_heat(config[CONF_SUPPORTS_HEAT]))
    cg.add(var.set_supports_cool(config[CONF_SUPPORTS_COOL]))
    cg.add(parent.register_climate_component(var))

