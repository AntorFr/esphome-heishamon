import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID, UNIT_CELSIUS
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon", "number"]

HeishamonNumber = heishamon_ns.class_("HeishamonNumber", cg.Component, number.Number)

# Available number entities based on HeishaMon protocol analysis
HEISHA_NUMBER_CONFIGS = {
    # Temperature controls (most important)
    "z1_heat_target_temp": {
        "name": "Zone 1 Heat Target Temperature",
        "command": "SetZ1HeatTargetTemp",
        "read_topic": "z1_heat_request_temp",  # Topic to read current value
        "min_value": 15.0,
        "max_value": 30.0,
        "step": 0.5,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "z2_heat_target_temp": {
        "name": "Zone 2 Heat Target Temperature", 
        "command": "SetZ2HeatTargetTemp",
        "read_topic": "z2_heat_request_temp",  # Topic to read current value
        "min_value": 15.0,
        "max_value": 30.0,
        "step": 0.5,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "z1_cool_target_temp": {
        "name": "Zone 1 Cool Target Temperature",
        "command": "SetZ1CoolTargetTemp", 
        "read_topic": "z1_cool_request_temp",  # Topic to read current value
        "min_value": 15.0,
        "max_value": 30.0,
        "step": 0.5,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "z2_cool_target_temp": {
        "name": "Zone 2 Cool Target Temperature",
        "command": "SetZ2CoolTargetTemp",
        "read_topic": "z2_cool_request_temp",  # Topic to read current value
        "min_value": 15.0,
        "max_value": 30.0, 
        "step": 0.5,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "dhw_target_temp": {
        "name": "DHW Target Temperature",
        "command": "SetDHWTargetTemp",
        "read_topic": "dhw_target_temp",  # Topic to read current value (exists!)
        "min_value": 40.0,
        "max_value": 65.0,
        "step": 1.0,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "main_target_temp": {
        "name": "Main Target Temperature", 
        "command": "SetMainTargetTemp",
        "read_topic": "main_target_temp",  # Topic to read current value (exists!)
        "min_value": 15.0,
        "max_value": 30.0,
        "step": 0.5,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    
    # Temperature offsets
    "room_temp_offset_z1": {
        "name": "Room Temperature Offset Zone 1",
        "command": "SetRoomTempOffsetZ1",
        "min_value": -5.0,
        "max_value": 5.0,
        "step": 0.1,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "room_temp_offset_z2": {
        "name": "Room Temperature Offset Zone 2", 
        "command": "SetRoomTempOffsetZ2",
        "min_value": -5.0,
        "max_value": 5.0,
        "step": 0.1,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    
    # Flow controls
    "z1_water_flow": {
        "name": "Zone 1 Water Flow Rate",
        "command": "SetZ1WaterFlow",
        "min_value": 5.0,
        "max_value": 25.0,
        "step": 1.0,
        "unit": "L/min",
        "device_class": None
    },
    "z2_water_flow": {
        "name": "Zone 2 Water Flow Rate",
        "command": "SetZ2WaterFlow", 
        "min_value": 5.0,
        "max_value": 25.0,
        "step": 1.0,
        "unit": "L/min",
        "device_class": None
    },
    
    # Pump controls
    "pump_speed": {
        "name": "Pump Speed",
        "command": "SetPumpSpeed",
        "min_value": 30.0,
        "max_value": 100.0,
        "step": 5.0,
        "unit": "%",
        "device_class": None
    },
    
    # Timing controls  
    "quiet_mode_start_time": {
        "name": "Quiet Mode Start Time",
        "command": "SetQuietModeStartTime",
        "min_value": 0.0,
        "max_value": 23.0,
        "step": 1.0,
        "unit": "h",
        "device_class": None
    },
    "quiet_mode_end_time": {
        "name": "Quiet Mode End Time",
        "command": "SetQuietModeEndTime",
        "min_value": 0.0,
        "max_value": 23.0,
        "step": 1.0,
        "unit": "h", 
        "device_class": None
    },
    "dhw_boost_time": {
        "name": "DHW Boost Time",
        "command": "SetDHWBoostTime",
        "min_value": 10.0,
        "max_value": 120.0,
        "step": 10.0,
        "unit": "min",
        "device_class": None
    },
    
    # Advanced controls
    "heating_curve_target_high": {
        "name": "Heating Curve Target High",
        "command": "SetHeatingCurveTargetHigh",
        "min_value": 25.0,
        "max_value": 55.0,
        "step": 1.0,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "heating_curve_target_low": {
        "name": "Heating Curve Target Low", 
        "command": "SetHeatingCurveTargetLow",
        "min_value": 25.0,
        "max_value": 55.0,
        "step": 1.0,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    
    # Delta controls (SetFloorHeatDelta, SetFloorCoolDelta, SetDHWHeatDelta, SetBufferDelta)
    "heat_delta": {
        "name": "Heat Delta",
        "command": "SetFloorHeatDelta",
        "read_topic": "heat_delta",
        "min_value": 1.0,
        "max_value": 15.0,
        "step": 1.0,
        "unit": "K",
        "device_class": None
    },
    "cool_delta": {
        "name": "Cool Delta",
        "command": "SetFloorCoolDelta",
        "read_topic": "cool_delta",
        "min_value": 1.0,
        "max_value": 15.0,
        "step": 1.0,
        "unit": "K",
        "device_class": None
    },
    "dhw_heat_delta": {
        "name": "DHW Heat Delta",
        "command": "SetDHWHeatDelta",
        "read_topic": "dhw_heat_delta",
        "min_value": 1.0,
        "max_value": 15.0,
        "step": 1.0,
        "unit": "K",
        "device_class": None
    },
    "buffer_tank_delta": {
        "name": "Buffer Tank Delta",
        "command": "SetBufferDelta",
        "read_topic": "buffer_tank_delta",
        "min_value": 1.0,
        "max_value": 15.0,
        "step": 1.0,
        "unit": "K",
        "device_class": None
    },
    
    # Pump duty
    "max_pump_duty": {
        "name": "Max Pump Duty",
        "command": "SetMaxPumpDuty",
        "read_topic": "max_pump_duty",
        "min_value": 50.0,
        "max_value": 100.0,
        "step": 1.0,
        "unit": "%",
        "device_class": None
    },
    
    # Outdoor temperature thresholds
    "heating_off_outdoor_temp": {
        "name": "Heating Off Outdoor Temperature",
        "command": "SetHeatingOffOutdoorTemp",
        "read_topic": "heating_off_outdoor_temp",
        "min_value": 5.0,
        "max_value": 35.0,
        "step": 1.0,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    
    # Bivalent controls
    "bivalent_start_temp": {
        "name": "Bivalent Start Temperature",
        "command": "SetBivalentStartTemp",
        "read_topic": "bivalent_start_temp",
        "min_value": -15.0,
        "max_value": 35.0,
        "step": 1.0,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "bivalent_ap_start_temp": {
        "name": "Bivalent AP Start Temperature",
        "command": "SetBivalentAPStartTemp",
        "read_topic": "bivalent_advanced_start_temp",
        "min_value": -15.0,
        "max_value": 35.0,
        "step": 1.0,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    "bivalent_ap_stop_temp": {
        "name": "Bivalent AP Stop Temperature",
        "command": "SetBivalentAPStopTemp",
        "read_topic": "bivalent_advanced_stop_temp",
        "min_value": -15.0,
        "max_value": 35.0,
        "step": 1.0,
        "unit": UNIT_CELSIUS,
        "device_class": "temperature"
    },
    
    # J-Series heater controls
    "heater_delay_time": {
        "name": "Heater Delay Time",
        "command": "SetHeaterDelayTime",
        "read_topic": "heater_delay_time",
        "min_value": 0.0,
        "max_value": 60.0,
        "step": 1.0,
        "unit": "min",
        "device_class": None
    },
    "heater_start_delta": {
        "name": "Heater Start Delta",
        "command": "SetHeaterStartDelta",
        "read_topic": "heater_start_delta",
        "min_value": 1.0,
        "max_value": 15.0,
        "step": 1.0,
        "unit": "K",
        "device_class": None
    },
    "heater_stop_delta": {
        "name": "Heater Stop Delta",
        "command": "SetHeaterStopDelta",
        "read_topic": "heater_stop_delta",
        "min_value": 1.0,
        "max_value": 15.0,
        "step": 1.0,
        "unit": "K",
        "device_class": None
    }
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HeishamonNumber),
        cv.GenerateID("heishamon_id"): cv.use_id(HeishamonComponent),
        cv.Required("type"): cv.one_of(*HEISHA_NUMBER_CONFIGS.keys(), lower=True),
        cv.Optional("min_value"): cv.float_,
        cv.Optional("max_value"): cv.float_, 
        cv.Optional("step"): cv.positive_float,
    }
).extend(number.number_schema(HeishamonNumber))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    
    # Get number configuration for this type
    number_config = HEISHA_NUMBER_CONFIGS[config["type"]]
    
    await number.register_number(
        var, 
        config,
        min_value=number_config["min_value"],
        max_value=number_config["max_value"], 
        step=number_config["step"]
    )
    
    # Register as Component for setup() to work
    await cg.register_component(var, config)

    parent = await cg.get_variable(config["heishamon_id"])
    cg.add(var.set_parent(parent))
    cg.add(var.set_number_type(config["type"]))
    
    # Configure number based on type
    number_config = HEISHA_NUMBER_CONFIGS[config["type"]]
    cg.add(var.set_command(number_config["command"]))
    
    # Set read topic if available for passive value display
    if "read_topic" in number_config:
        cg.add(var.set_read_topic(number_config["read_topic"]))
    
    # Use provided values or defaults from config
    min_val = config.get("min_value", number_config["min_value"])
    max_val = config.get("max_value", number_config["max_value"])
    step_val = config.get("step", number_config["step"])
    
    cg.add(var.set_min_value(min_val))
    cg.add(var.set_max_value(max_val))
    cg.add(var.set_step(step_val))
    
    # Set unit and device class if specified
    if number_config["unit"]:
        cg.add(var.set_unit_of_measurement(number_config["unit"]))
    if number_config["device_class"]:
        cg.add(var.set_device_class(number_config["device_class"]))
