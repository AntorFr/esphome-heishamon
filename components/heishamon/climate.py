import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import (
    CONF_ID,
    CONF_NAME,
)

from . import CONF_HEISHAMON_ID, heishamon_ns, HeishamonComponent

DEPENDENCIES = ["heishamon"]

CONF_ZONE_ID = "zone_id"
CONF_SUPPORTS_HEAT = "supports_heat"
CONF_SUPPORTS_COOL = "supports_cool"
CONF_TYPE = "type"

HeishaMonClimate = heishamon_ns.class_(
    "HeishaMonClimate", climate.Climate, cg.Component
)
HeishamonWaterHeater = heishamon_ns.class_(
    "HeishamonWaterHeater", climate.Climate, cg.Component
)

HeishamonWaterHeater = heishamon_ns.class_(
    "HeishamonWaterHeater", climate.Climate, cg.Component
)

# Configuration schema for climate components - conditional based on type
def _conditional_schema(config):
    if isinstance(config, dict) and config.get(CONF_TYPE) == "water_heater":
        # For water heater type, use HeishamonWaterHeater
        return climate.climate_schema(HeishamonWaterHeater).extend({
            cv.GenerateID(CONF_HEISHAMON_ID): cv.use_id(HeishamonComponent),
            cv.Optional(CONF_TYPE): cv.one_of("water_heater", lower=True),
        }).extend(cv.COMPONENT_SCHEMA)(config)
    else:
        # For regular climate, use HeishaMonClimate
        return climate.climate_schema(HeishaMonClimate).extend({
            cv.GenerateID(CONF_HEISHAMON_ID): cv.use_id(HeishamonComponent),
            cv.Optional(CONF_ZONE_ID): cv.int_range(min=1, max=2),
            cv.Optional(CONF_SUPPORTS_HEAT, default=True): cv.boolean,
            cv.Optional(CONF_SUPPORTS_COOL, default=False): cv.boolean,
            cv.Optional(CONF_TYPE): cv.one_of("water_heater", lower=True),
        }).extend(cv.COMPONENT_SCHEMA)(config)

CONFIG_SCHEMA = _conditional_schema


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    parent = await cg.get_variable(config[CONF_HEISHAMON_ID])
    
    # Check if this is a water_heater type
    if config.get(CONF_TYPE) == "water_heater":
        cg.add(parent.register_water_heater(var))
    else:
        # Regular zone climate configuration
        cg.add(var.set_parent(parent))
        
        # Zone ID is required for regular climate
        if CONF_ZONE_ID not in config:
            raise cv.Invalid("zone_id is required for regular climate entities")
        
        cg.add(var.set_zone_id(config[CONF_ZONE_ID]))
        cg.add(var.set_supports_heat(config[CONF_SUPPORTS_HEAT]))
        cg.add(var.set_supports_cool(config[CONF_SUPPORTS_COOL]))

        cg.add(parent.register_climate_component(var))
