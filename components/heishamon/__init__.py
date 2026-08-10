import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ['uart']
CODEOWNERS = ["@AntorFR"]
MULTI_CONF = True

CONF_HEISHAMON_ID = "heishamon_id"

heishamon_ns = cg.esphome_ns.namespace('heishamon')
HeishamonComponent = heishamon_ns.class_('HeishamonComponent', cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(HeishamonComponent),
        cv.Optional("update_interval", default="30s"): cv.positive_time_period_milliseconds,
        cv.Optional("listen_only", default=False): cv.boolean,
        cv.Optional("optional_pcb", default=False): cv.boolean,
        # TX driver enable (HeishaMon ESP32 boards gate the CN-CNT TX stage
        # behind GPIO5): driven HIGH in active mode, LOW in listen-only mode
        cv.Optional("tx_enable_pin"): pins.gpio_output_pin_schema,
    }).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    
    cg.add(var.set_update_interval(config["update_interval"]))
    cg.add(var.set_listen_only(config["listen_only"]))
    cg.add(var.set_optional_pcb(config["optional_pcb"]))

    if "tx_enable_pin" in config:
        tx_enable = await cg.gpio_pin_expression(config["tx_enable_pin"])
        cg.add(var.set_tx_enable_pin(tx_enable))
