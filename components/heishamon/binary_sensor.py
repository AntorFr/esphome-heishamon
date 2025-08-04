import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon"]

HeishamonBinarySensor = heishamon_ns.class_("HeishamonBinarySensor", binary_sensor.BinarySensor, cg.Component)

# Topics pour binary sensors
HEISHA_BINARY_TOPICS = {
    "heatpump_state": "Heat pump state",
    "defrosting_state": "Defrosting state", 
    "dhw_heater_state": "DHW heater state",
    "room_heater_state": "Room heater state",
    "sterilization_state": "Sterilization state",
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HeishamonBinarySensor),
        cv.GenerateID("heishamon_id"): cv.use_id(HeishamonComponent),
        cv.Required("topic"): cv.one_of(*HEISHA_BINARY_TOPICS.keys(), lower=True),
    }
).extend(binary_sensor.binary_sensor_schema())

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)

    parent = await cg.get_variable(config["heishamon_id"])
    cg.add(var.set_parent(parent))
    cg.add(var.set_topic(config["topic"]))
