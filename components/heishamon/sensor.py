import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_FREQUENCY, 
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_PRESSURE,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLUME_FLOW_RATE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_HERTZ,
    UNIT_WATT,
)
from . import HeishamonComponent, heishamon_ns

DEPENDENCIES = ["heishamon"]

HeishamonSensor = heishamon_ns.class_("HeishamonSensor", sensor.Sensor)

# Unit aliases not in esphome.const
UNIT_LITER_PER_MIN = "l/min"
UNIT_HOUR = "h"
UNIT_MINUTE = "min"
UNIT_KELVIN = "K"
UNIT_PERCENT = "%"
UNIT_BAR = "bar"
UNIT_KGF_CM2 = "kgf/cm²"
UNIT_RPM = "r/min"

# Complete topics configuration based on HeishaMon protocol
# Reference: https://github.com/Egyras/HeishaMon
HEISHA_TOPICS = {
    # ============== BASIC STATE SENSORS (TOP0-TOP14) ==============
    
    # TOP1 - Pump Flow
    "pump_flow": {
        "name": "Pump Flow",
        "topic_id": 1,
        "unit": UNIT_LITER_PER_MIN, 
        "device_class": DEVICE_CLASS_VOLUME_FLOW_RATE,
        "icon": "mdi:pump",
        "accuracy_decimals": 2
    },
    
    # TOP4 - Operating Mode State (numeric value, use select for text)
    "operation_mode": {
        "name": "Operating Mode State",
        "topic_id": 4,
        "unit": "", 
        "device_class": None,
        "icon": "mdi:heat-pump"
    },
    
    # ============== TEMPERATURE SENSORS ==============
    
    # TOP5 - Main Inlet Temperature
    "main_inlet_temp": {
        "name": "Main Inlet Temperature",
        "topic_id": 5,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-water"
    },
    
    # TOP6 - Main Outlet Temperature
    "main_outlet_temp": {
        "name": "Main Outlet Temperature",
        "topic_id": 6,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-water"
    },
    
    # TOP7 - Main Target Temperature
    "main_target_temp": {
        "name": "Main Target Temperature",
        "topic_id": 7,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP9 - DHW Target Temperature
    "dhw_target_temp": {
        "name": "DHW Target Temperature",
        "topic_id": 9,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP10 - DHW Temperature
    "dhw_temp": {
        "name": "DHW Temperature",
        "topic_id": 10,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:water-thermometer"
    },
    
    # TOP14 - Outside Temperature
    "outside_temp": {
        "name": "Outside Temperature",
        "topic_id": 14,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:home-thermometer-outline"
    },
    
    # TOP21 - Outside Pipe Temperature
    "outside_pipe_temp": {
        "name": "Outside Pipe Temperature",
        "topic_id": 21,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:pipe"
    },
    
    # TOP33 - Room Thermostat Temperature
    "room_thermostat_temp": {
        "name": "Room Thermostat Temperature",
        "topic_id": 33,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:home-thermometer"
    },
    
    # TOP36 - Zone 1 Water Temperature
    "z1_water_temp": {
        "name": "Zone 1 Water Temperature",
        "topic_id": 36,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-water"
    },
    
    # TOP37 - Zone 2 Water Temperature
    "z2_water_temp": {
        "name": "Zone 2 Water Temperature",
        "topic_id": 37,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-water"
    },
    
    # TOP42 - Zone 1 Water Target Temperature
    "z1_water_target_temp": {
        "name": "Zone 1 Water Target Temperature",
        "topic_id": 42,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-water"
    },
    
    # TOP43 - Zone 2 Water Target Temperature
    "z2_water_target_temp": {
        "name": "Zone 2 Water Target Temperature",
        "topic_id": 43,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-water"
    },
    
    # TOP46 - Buffer Tank Temperature
    "buffer_temp": {
        "name": "Buffer Temperature",
        "topic_id": 46,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:storage-tank"
    },
    
    # TOP47 - Solar Temperature
    "solar_temp": {
        "name": "Solar Temperature",
        "topic_id": 47,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:solar-power"
    },
    
    # TOP48 - Pool Temperature
    "pool_temp": {
        "name": "Pool Temperature",
        "topic_id": 48,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:pool-thermometer"
    },
    
    # TOP49 - Main Heat Exchanger Outlet Temperature
    "main_hex_outlet_temp": {
        "name": "Main HEX Outlet Temperature",
        "topic_id": 49,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP50 - Discharge Temperature
    "discharge_temp": {
        "name": "Discharge Temperature",
        "topic_id": 50,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-high"
    },
    
    # TOP51 - Inside Pipe Temperature
    "inside_pipe_temp": {
        "name": "Inside Pipe Temperature",
        "topic_id": 51,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:pipe"
    },
    
    # TOP52 - Defrost Temperature
    "defrost_temp": {
        "name": "Defrost Temperature",
        "topic_id": 52,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:snowflake-melt"
    },
    
    # TOP53 - Evaporator Outlet Temperature
    "eva_outlet_temp": {
        "name": "Evaporator Outlet Temperature",
        "topic_id": 53,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP54 - Bypass Outlet Temperature
    "bypass_outlet_temp": {
        "name": "Bypass Outlet Temperature",
        "topic_id": 54,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP55 - IPM Temperature
    "ipm_temp": {
        "name": "IPM Temperature",
        "topic_id": 55,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chip"
    },
    
    # TOP56 - Zone 1 Temperature
    "z1_temp": {
        "name": "Zone 1 Temperature",
        "topic_id": 56,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:home-thermometer"
    },
    
    # TOP57 - Zone 2 Temperature
    "z2_temp": {
        "name": "Zone 2 Temperature",
        "topic_id": 57,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:home-thermometer"
    },
    
    # TOP70 - Sterilization Temperature
    "sterilization_temp": {
        "name": "Sterilization Temperature",
        "topic_id": 70,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:bacteria-outline"
    },
    
    # TOP116 - Second Inlet Temperature
    "second_inlet_temp": {
        "name": "Second Inlet Temperature",
        "topic_id": 116,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP117 - Economizer Outlet Temperature
    "economizer_outlet_temp": {
        "name": "Economizer Outlet Temperature",
        "topic_id": 117,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP118 - Second Room Thermostat Temperature
    "second_room_thermostat_temp": {
        "name": "Second Room Thermostat Temperature",
        "topic_id": 118,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:home-thermometer"
    },
    
    # ============== COMPRESSOR & MOTOR SENSORS ==============
    
    # TOP8 - Compressor Frequency
    "compressor_freq": {
        "name": "Compressor Frequency",
        "topic_id": 8,
        "unit": UNIT_HERTZ, 
        "device_class": DEVICE_CLASS_FREQUENCY,
        "icon": "mdi:sine-wave"
    },
    
    # TOP62 - Fan 1 Motor Speed
    "fan1_motor_speed": {
        "name": "Fan 1 Motor Speed",
        "topic_id": 62,
        "unit": UNIT_RPM, 
        "device_class": None,
        "icon": "mdi:fan"
    },
    
    # TOP63 - Fan 2 Motor Speed
    "fan2_motor_speed": {
        "name": "Fan 2 Motor Speed",
        "topic_id": 63,
        "unit": UNIT_RPM, 
        "device_class": None,
        "icon": "mdi:fan"
    },
    
    # TOP65 - Pump Speed
    "pump_speed": {
        "name": "Pump Speed",
        "topic_id": 65,
        "unit": UNIT_RPM, 
        "device_class": None,
        "icon": "mdi:pump"
    },
    
    # TOP67 - Compressor Current
    "compressor_current": {
        "name": "Compressor Current",
        "topic_id": 67,
        "unit": UNIT_AMPERE, 
        "device_class": DEVICE_CLASS_CURRENT,
        "icon": "mdi:current-ac"
    },
    
    # ============== PRESSURE SENSORS ==============
    
    # TOP64 - High Pressure
    "high_pressure": {
        "name": "High Pressure",
        "topic_id": 64,
        "unit": UNIT_KGF_CM2, 
        "device_class": DEVICE_CLASS_PRESSURE,
        "icon": "mdi:gauge"
    },
    
    # TOP66 - Low Pressure
    "low_pressure": {
        "name": "Low Pressure",
        "topic_id": 66,
        "unit": UNIT_KGF_CM2, 
        "device_class": DEVICE_CLASS_PRESSURE,
        "icon": "mdi:gauge-low"
    },
    
    # TOP115 - Water Pressure
    "water_pressure": {
        "name": "Water Pressure",
        "topic_id": 115,
        "unit": UNIT_BAR, 
        "device_class": DEVICE_CLASS_PRESSURE,
        "icon": "mdi:gauge",
        "accuracy_decimals": 2
    },
    
    # ============== POWER SENSORS ==============
    
    # TOP15/TOP38 - Heat Power Production
    "heat_power_production": {
        "name": "Heat Power Production",
        "topic_id": 15,
        "unit": UNIT_WATT, 
        "device_class": DEVICE_CLASS_POWER,
        "icon": "mdi:fire"
    },
    
    # TOP16/TOP39 - Heat Power Consumption
    "heat_power_consumption": {
        "name": "Heat Power Consumption",
        "topic_id": 16,
        "unit": UNIT_WATT, 
        "device_class": DEVICE_CLASS_POWER,
        "icon": "mdi:flash"
    },
    
    # TOP40 - DHW Power Production
    "dhw_power_production": {
        "name": "DHW Power Production",
        "topic_id": 40,
        "unit": UNIT_WATT, 
        "device_class": DEVICE_CLASS_POWER,
        "icon": "mdi:water-boiler"
    },
    
    # TOP41 - DHW Power Consumption
    "dhw_power_consumption": {
        "name": "DHW Power Consumption",
        "topic_id": 41,
        "unit": UNIT_WATT, 
        "device_class": DEVICE_CLASS_POWER,
        "icon": "mdi:flash"
    },
    
    # XTOP1/XTOP4 - Cool Power
    "cool_power_production": {
        "name": "Cool Power Production",
        "topic_id": "XTOP4",
        "unit": UNIT_WATT, 
        "device_class": DEVICE_CLASS_POWER,
        "icon": "mdi:snowflake"
    },
    
    "cool_power_consumption": {
        "name": "Cool Power Consumption",
        "topic_id": "XTOP1",
        "unit": UNIT_WATT, 
        "device_class": DEVICE_CLASS_POWER,
        "icon": "mdi:flash"
    },
    
    # ============== DELTA SENSORS ==============
    
    # TOP22 - DHW Heat Delta
    "dhw_heat_delta": {
        "name": "DHW Heat Delta",
        "topic_id": 22,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:delta"
    },
    
    # TOP23 - Heat Delta
    "heat_delta": {
        "name": "Heat Delta",
        "topic_id": 23,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:delta"
    },
    
    # TOP24 - Cool Delta
    "cool_delta": {
        "name": "Cool Delta",
        "topic_id": 24,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:delta"
    },
    
    # TOP113 - Buffer Tank Delta
    "buffer_tank_delta": {
        "name": "Buffer Tank Delta",
        "topic_id": 113,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:delta"
    },
    
    # ============== OPERATING HOURS & COUNTERS ==============
    
    # TOP11 - Operations Hours
    "operations_hours": {
        "name": "Operations Hours",
        "topic_id": 11,
        "unit": UNIT_HOUR, 
        "device_class": DEVICE_CLASS_DURATION,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
        "icon": "mdi:clock-outline"
    },
    
    # TOP12 - Operations Counter
    "operations_counter": {
        "name": "Operations Counter",
        "topic_id": 12,
        "unit": "", 
        "device_class": None,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
        "icon": "mdi:counter"
    },
    
    # TOP90 - Room Heater Operations Hours
    "room_heater_operations_hours": {
        "name": "Room Heater Operations Hours",
        "topic_id": 90,
        "unit": UNIT_HOUR, 
        "device_class": DEVICE_CLASS_DURATION,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
        "icon": "mdi:radiator"
    },
    
    # TOP91 - DHW Heater Operations Hours
    "dhw_heater_operations_hours": {
        "name": "DHW Heater Operations Hours",
        "topic_id": 91,
        "unit": UNIT_HOUR, 
        "device_class": DEVICE_CLASS_DURATION,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
        "icon": "mdi:water-boiler"
    },
    
    # TOP71 - Sterilization Max Time
    "sterilization_max_time": {
        "name": "Sterilization Max Time",
        "topic_id": 71,
        "unit": UNIT_MINUTE, 
        "device_class": DEVICE_CLASS_DURATION,
        "icon": "mdi:bacteria-outline"
    },
    
    # ============== ZONE REQUEST TEMPERATURES ==============
    
    # TOP27 - Zone 1 Heat Request Temperature
    "z1_heat_request_temp": {
        "name": "Zone 1 Heat Request Temperature",
        "topic_id": 27,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP28 - Zone 1 Cool Request Temperature
    "z1_cool_request_temp": {
        "name": "Zone 1 Cool Request Temperature",
        "topic_id": 28,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP29 - Zone 2 Heat Request Temperature
    "z2_heat_request_temp": {
        "name": "Zone 2 Heat Request Temperature",
        "topic_id": 29,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # TOP30 - Zone 2 Cool Request Temperature
    "z2_cool_request_temp": {
        "name": "Zone 2 Cool Request Temperature",
        "topic_id": 30,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer"
    },
    
    # ============== CURVE TEMPERATURES (Zone 1 Heat) ==============
    
    # TOP31-34 - Zone 1 Heat Curve
    "z1_heat_curve_target_high": {
        "name": "Zone 1 Heat Curve Target High",
        "topic_id": 31,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z1_heat_curve_target_low": {
        "name": "Zone 1 Heat Curve Target Low",
        "topic_id": 32,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z1_heat_curve_outside_high": {
        "name": "Zone 1 Heat Curve Outside High",
        "topic_id": 33,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z1_heat_curve_outside_low": {
        "name": "Zone 1 Heat Curve Outside Low",
        "topic_id": 34,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    # TOP72-75 - Zone 1 Cool Curve
    "z1_cool_curve_target_high": {
        "name": "Zone 1 Cool Curve Target High",
        "topic_id": 72,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z1_cool_curve_target_low": {
        "name": "Zone 1 Cool Curve Target Low",
        "topic_id": 73,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z1_cool_curve_outside_high": {
        "name": "Zone 1 Cool Curve Outside High",
        "topic_id": 74,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z1_cool_curve_outside_low": {
        "name": "Zone 1 Cool Curve Outside Low",
        "topic_id": 75,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    # ============== CURVE TEMPERATURES (Zone 2) ==============
    
    # TOP36-37 + TOP82-85 - Zone 2 Heat Curve
    "z2_heat_curve_target_high": {
        "name": "Zone 2 Heat Curve Target High",
        "topic_id": 82,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z2_heat_curve_target_low": {
        "name": "Zone 2 Heat Curve Target Low",
        "topic_id": 83,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z2_heat_curve_outside_high": {
        "name": "Zone 2 Heat Curve Outside High",
        "topic_id": 84,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z2_heat_curve_outside_low": {
        "name": "Zone 2 Heat Curve Outside Low",
        "topic_id": 85,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    # TOP86-89 - Zone 2 Cool Curve
    "z2_cool_curve_target_high": {
        "name": "Zone 2 Cool Curve Target High",
        "topic_id": 86,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z2_cool_curve_target_low": {
        "name": "Zone 2 Cool Curve Target Low",
        "topic_id": 87,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z2_cool_curve_outside_high": {
        "name": "Zone 2 Cool Curve Outside High",
        "topic_id": 88,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    "z2_cool_curve_outside_low": {
        "name": "Zone 2 Cool Curve Outside Low",
        "topic_id": 89,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:chart-bell-curve"
    },
    
    # ============== AUTO MODE TEMPERATURES ==============
    
    # TOP77-80 - Outdoor Temperature Thresholds
    "heating_off_outdoor_temp": {
        "name": "Heating Off Outdoor Temperature",
        "topic_id": 77,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-off"
    },
    
    "heater_on_outdoor_temp": {
        "name": "Heater On Outdoor Temperature",
        "topic_id": 78,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:radiator"
    },
    
    "heat_to_cool_temp": {
        "name": "Heat to Cool Temperature",
        "topic_id": 79,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-chevron-up"
    },
    
    "cool_to_heat_temp": {
        "name": "Cool to Heat Temperature",
        "topic_id": 80,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:thermometer-chevron-down"
    },
    
    # ============== HOLIDAY TEMPERATURES ==============
    
    # TOP25 - DHW Holiday Shift
    "dhw_holiday_shift_temp": {
        "name": "DHW Holiday Shift Temperature",
        "topic_id": 25,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:beach"
    },
    
    # TOP45 - Room Holiday Shift
    "room_holiday_shift_temp": {
        "name": "Room Holiday Shift Temperature",
        "topic_id": 45,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:beach"
    },
    
    # ============== PUMP & VALVE SENSORS ==============
    
    # TOP93 - Pump Duty
    "pump_duty": {
        "name": "Pump Duty",
        "topic_id": 93,
        "unit": "", 
        "device_class": None,
        "icon": "mdi:pump"
    },
    
    # TOP95 - Max Pump Duty
    "max_pump_duty": {
        "name": "Max Pump Duty",
        "topic_id": 95,
        "unit": "", 
        "device_class": None,
        "icon": "mdi:pump"
    },
    
    # TOP127 - Zone 1 Valve PID
    "z1_valve_pid": {
        "name": "Zone 1 Valve PID",
        "topic_id": 127,
        "unit": UNIT_PERCENT, 
        "device_class": None,
        "icon": "mdi:valve"
    },
    
    # TOP128 - Zone 2 Valve PID
    "z2_valve_pid": {
        "name": "Zone 2 Valve PID",
        "topic_id": 128,
        "unit": UNIT_PERCENT, 
        "device_class": None,
        "icon": "mdi:valve"
    },
    
    # ============== SOLAR SENSORS ==============
    
    # TOP102-105 - Solar Settings
    "solar_on_delta": {
        "name": "Solar On Delta",
        "topic_id": 102,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:solar-power"
    },
    
    "solar_off_delta": {
        "name": "Solar Off Delta",
        "topic_id": 103,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:solar-power"
    },
    
    "solar_frost_protection": {
        "name": "Solar Frost Protection",
        "topic_id": 104,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:snowflake-alert"
    },
    
    "solar_high_limit": {
        "name": "Solar High Limit",
        "topic_id": 105,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:solar-power"
    },
    
    # ============== BIVALENT SENSORS ==============
    
    # TOP131-138 - Bivalent Settings
    "bivalent_start_temp": {
        "name": "Bivalent Start Temperature",
        "topic_id": 131,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:heat-wave"
    },
    
    "bivalent_advanced_start_temp": {
        "name": "Bivalent Advanced Start Temperature",
        "topic_id": 134,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:heat-wave"
    },
    
    "bivalent_advanced_stop_temp": {
        "name": "Bivalent Advanced Stop Temperature",
        "topic_id": 135,
        "unit": UNIT_CELSIUS, 
        "device_class": DEVICE_CLASS_TEMPERATURE,
        "icon": "mdi:heat-wave"
    },
    
    "bivalent_advanced_start_delay": {
        "name": "Bivalent Advanced Start Delay",
        "topic_id": 136,
        "unit": UNIT_MINUTE, 
        "device_class": DEVICE_CLASS_DURATION,
        "icon": "mdi:timer"
    },
    
    "bivalent_advanced_stop_delay": {
        "name": "Bivalent Advanced Stop Delay",
        "topic_id": 137,
        "unit": UNIT_MINUTE, 
        "device_class": DEVICE_CLASS_DURATION,
        "icon": "mdi:timer"
    },
    
    "bivalent_advanced_dhw_delay": {
        "name": "Bivalent Advanced DHW Delay",
        "topic_id": 138,
        "unit": UNIT_MINUTE, 
        "device_class": DEVICE_CLASS_DURATION,
        "icon": "mdi:timer"
    },
    
    # ============== J-SERIES SPECIFIC ==============
    
    # TOP96-98 - Heater Settings (J-series)
    "heater_delay_time": {
        "name": "Heater Delay Time",
        "topic_id": 96,
        "unit": UNIT_MINUTE, 
        "device_class": DEVICE_CLASS_DURATION,
        "icon": "mdi:timer"
    },
    
    "heater_start_delta": {
        "name": "Heater Start Delta",
        "topic_id": 97,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:delta"
    },
    
    "heater_stop_delta": {
        "name": "Heater Stop Delta",
        "topic_id": 98,
        "unit": UNIT_KELVIN, 
        "device_class": None,
        "icon": "mdi:delta"
    },
    
    # ============== CALCULATED ==============
    
    # COP (coefficient of performance) - calculated from power data
    "cop": {
        "name": "COP",
        "topic_id": None,
        "unit": "", 
        "device_class": None,
        "icon": "mdi:gauge"
    },
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
    await sensor.register_sensor(var, config)

    parent = await cg.get_variable(config["heishamon_id"])
    
    # Register callback with parent component for this topic
    topic = config["topic"]
    cg.add(parent.register_sensor_callback(
        topic,
        cg.RawExpression(f"[=](float value) {{ {var}->publish_state(value); }}")
    ))
    
    # Automatic configuration of units and classes
    topic_config = HEISHA_TOPICS[config["topic"]]
    if topic_config["unit"] and "unit_of_measurement" not in config:
        cg.add(var.set_unit_of_measurement(topic_config["unit"]))
    if topic_config.get("device_class") and "device_class" not in config:
        cg.add(var.set_device_class(topic_config["device_class"]))
    # Note: state_class should be set through the config schema, not manually here
    if topic_config.get("icon") and "icon" not in config:
        cg.add(var.set_icon(topic_config["icon"]))
    if topic_config.get("accuracy_decimals") is not None and "accuracy_decimals" not in config:
        cg.add(var.set_accuracy_decimals(topic_config["accuracy_decimals"]))
