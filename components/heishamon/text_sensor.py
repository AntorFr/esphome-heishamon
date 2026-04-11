import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    CONF_ID,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import HeishamonComponent, heishamon_ns, CONF_HEISHAMON_ID

DEPENDENCIES = ["heishamon", "text_sensor"]

HeishamonTextSensor = heishamon_ns.class_("HeishamonTextSensor", text_sensor.TextSensor)

CONF_TOPIC = "topic"

# Text sensor topics based on HeishaMon protocol
HEISHA_TEXT_TOPICS = {
    # TOP44 - Error Code
    "error": {
        "name": "Error Code",
        "topic_id": 44,
        "icon": "mdi:alert-circle",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
    
    # TOP92 - Heat Pump Model (decoded from multiple bytes)
    "heat_pump_model": {
        "name": "Heat Pump Model",
        "topic_id": 92,
        "icon": "mdi:heat-pump",
        "entity_category": ENTITY_CATEGORY_DIAGNOSTIC,
    },
}

def get_topic_config(topic_name):
    """Get configuration for a specific topic."""
    return HEISHA_TEXT_TOPICS.get(topic_name, {
        "name": topic_name.replace("_", " ").title(),
        "topic_id": -1,
        "icon": "mdi:text",
        "entity_category": None,
    })


CONFIG_SCHEMA = cv.All(
    text_sensor.text_sensor_schema(HeishamonTextSensor)
    .extend({
        cv.GenerateID(CONF_HEISHAMON_ID): cv.use_id(HeishamonComponent),
        cv.Required(CONF_TOPIC): cv.string,
    })
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_HEISHAMON_ID])
    topic = config[CONF_TOPIC]
    
    # Get topic-specific configuration
    topic_config = get_topic_config(topic)
    
    # Merge icon into config for registration
    merged_config = dict(config)
    if "icon" in topic_config and "icon" not in merged_config:
        merged_config["icon"] = topic_config["icon"]
    
    # Create the text sensor
    var = await text_sensor.new_text_sensor(merged_config)
    
    # Register with parent component
    cg.add(parent.register_text_sensor_callback(
        topic,
        cg.RawExpression(f"[=](const std::string &value) {{ {var}->publish_state(value); }}")
    ))
