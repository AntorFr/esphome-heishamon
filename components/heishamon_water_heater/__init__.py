from ..heishamon import water_heater

DEPENDENCIES = ["heishamon"]

CONFIG_SCHEMA = water_heater.CONFIG_SCHEMA

async def to_code(config):
    await water_heater.to_code(config)
