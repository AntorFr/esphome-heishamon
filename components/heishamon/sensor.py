import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_FREQUENCY, 
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLUME_FLOW_RATE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_CUBIC_METER_PER_HOUR,
    UNIT_HERTZ,
    UNIT_WATT,
)
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon"]

HeishamonSensor = heishamon_ns.class_("HeishamonSensor", sensor.Sensor, cg.Component)

# Available topics configuration
HEISHA_TOPICS = {
    "heatpump_state": {"unit": "", "device_class": None, "state_class": STATE_CLASS_MEASUREMENT},
    "main_inlet_temp": {"unit": UNIT_CELSIUS, "device_class": DEVICE_CLASS_TEMPERATURE, "state_class": STATE_CLASS_MEASUREMENT},
    "main_outlet_temp": {"unit": UNIT_CELSIUS, "device_class": DEVICE_CLASS_TEMPERATURE, "state_class": STATE_CLASS_MEASUREMENT},
    "main_target_temp": {"unit": UNIT_CELSIUS, "device_class": DEVICE_CLASS_TEMPERATURE, "state_class": STATE_CLASS_MEASUREMENT},
    "dhw_temp": {"unit": UNIT_CELSIUS, "device_class": DEVICE_CLASS_TEMPERATURE, "state_class": STATE_CLASS_MEASUREMENT},
    "dhw_target_temp": {"unit": UNIT_CELSIUS, "device_class": DEVICE_CLASS_TEMPERATURE, "state_class": STATE_CLASS_MEASUREMENT},
    "outside_temp": {"unit": UNIT_CELSIUS, "device_class": DEVICE_CLASS_TEMPERATURE, "state_class": STATE_CLASS_MEASUREMENT},
    "compressor_freq": {"unit": UNIT_HERTZ, "device_class": DEVICE_CLASS_FREQUENCY, "state_class": STATE_CLASS_MEASUREMENT},
    "heat_power_production": {"unit": UNIT_WATT, "device_class": DEVICE_CLASS_POWER, "state_class": STATE_CLASS_MEASUREMENT},
    "heat_power_consumption": {"unit": UNIT_WATT, "device_class": DEVICE_CLASS_POWER, "state_class": STATE_CLASS_MEASUREMENT},
    "pump_flow": (None, UNIT_CUBIC_METER_PER_HOUR, DEVICE_CLASS_VOLUME_FLOW_RATE),
    "operation_mode": {"unit": "", "device_class": None, "state_class": STATE_CLASS_MEASUREMENT},
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HeishamonSensor),
        cv.GenerateID("heishamon_id"): cv.use_id(HeishamonComponent),
        cv.Required("topic"): cv.one_of(*HEISHA_TOPICS.keys(), lower=True),
    }
).extend(sensor.sensor_schema())

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    parent = await cg.get_variable(config["heishamon_id"])
    cg.add(var.set_parent(parent))
    cg.add(var.set_topic(config["topic"]))
    
    # Automatic configuration of units and classes
    topic_config = HEISHA_TOPICS[config["topic"]]
    if topic_config["unit"] and "unit_of_measurement" not in config:
        cg.add(var.set_unit_of_measurement(topic_config["unit"]))
    if topic_config["device_class"] and "device_class" not in config:
        cg.add(var.set_device_class(topic_config["device_class"]))
    if topic_config["state_class"] and "state_class" not in config:
        cg.add(var.set_state_class(topic_config["state_class"]))
