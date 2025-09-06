import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    UNIT_CELSIUS,
)
from . import HeishamonComponent, CONF_HEISHAMON_ID, heishamon_ns

DEPENDENCIES = ["heishamon"]

# Water Heater Climate Component
HeishamonWaterHeater = heishamon_ns.class_(
    "HeishamonWaterHeater", climate.Climate, cg.Component
)

# Configuration schema for Water Heater
CONFIG_SCHEMA = climate.climate_schema(HeishamonWaterHeater).extend({
    cv.GenerateID(CONF_HEISHAMON_ID): cv.use_id(HeishamonComponent),
    cv.Optional(CONF_NAME, default="DHW"): cv.string,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate code for Water Heater Climate component."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    # Get parent HeishaMon component
    parent = await cg.get_variable(config[CONF_HEISHAMON_ID])
    
    # Register this water heater with parent component
    cg.add(parent.register_water_heater(var))
