#include "heishamon.h"
#include "esphome/core/log.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon";

HeishamonComponent::HeishamonComponent() {
  ESP_LOGD(TAG, "HeishamonComponent constructor - creating callback manager");
  this->callback_manager_ = new HeishamonCallbackManager();
}

void HeishamonComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heishamon...");
  
  ESP_LOGCONFIG(TAG, "Callback manager has %d sensor callbacks, %d binary sensor callbacks, %d switch callbacks, %d select callbacks", 
               this->callback_manager_->get_sensor_callback_count(),
               this->callback_manager_->get_binary_sensor_callback_count(),
               this->callback_manager_->get_switch_callback_count(),
               this->callback_manager_->get_select_callback_count());
  
  // Initialize protocol layer with UART component
  this->protocol_ = new HeishamonProtocol(this);
  this->protocol_->set_listen_only(this->listen_only_);
  this->protocol_->set_optional_pcb(this->optional_pcb_);
  
  // Set callback to receive data from protocol layer
  this->protocol_->set_data_callback([this](const std::vector<uint8_t> &data, uint8_t data_type) {
    this->on_protocol_data_received(data, data_type);
  });
  
  this->protocol_->init();
  
  ESP_LOGCONFIG(TAG, "Heishamon setup completed");
}

void HeishamonComponent::loop() {
  uint32_t now = millis();
  
  // Let protocol handle all serial communication
  if (this->protocol_) {
    this->protocol_->process_loop();
  }
  
  // Send periodic queries
  if ((now - this->last_run_time_) > this->update_interval_) {
    this->last_run_time_ = now;
    
    if (!this->listen_only_ && this->protocol_) {
      // Send initial query first time only
      static bool initial_query_sent = false;
      if (!initial_query_sent) {
        ESP_LOGI(TAG, "Sending initial query to heatpump");
        this->protocol_->send_initial_query();
        initial_query_sent = true;
      } else {
        this->protocol_->send_panasonic_query();
        
        // Send optional PCB query if enabled
        if (this->optional_pcb_ && (now - this->last_optional_pcb_time_) > 1000) {
          this->last_optional_pcb_time_ = now;
          this->protocol_->send_optional_pcb_query();
        }
      }
    }
  }
}

void HeishamonComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Heishamon:");
  ESP_LOGCONFIG(TAG, "  Update interval: %u ms", this->update_interval_);
  ESP_LOGCONFIG(TAG, "  Listen only: %s", YESNO(this->listen_only_));
  ESP_LOGCONFIG(TAG, "  Optional PCB: %s", YESNO(this->optional_pcb_));
}

void HeishamonComponent::on_protocol_data_received(const std::vector<uint8_t> &data, uint8_t data_type) {
  ESP_LOGV(TAG, "Received data from protocol layer: %d bytes, type: 0x%02X", data.size(), data_type);
  
  if (data_type == 0x10) {  // DATA_TYPE_NORMAL
    ESP_LOGV(TAG, "Processing standard heatpump data");
    this->decode_and_notify_sensors(data);
  } else if (data_type == 0x21) {  // DATA_TYPE_EXTRA  
    ESP_LOGV(TAG, "Processing extra heatpump data");
    // TODO: Implement extra data processing
  } else if (data_type == 0xF1) {  // Optional PCB data
    ESP_LOGV(TAG, "Processing optional PCB data");
    // TODO: Implement optional PCB data processing
  }
}

void HeishamonComponent::register_sensor_callback(const std::string &topic, std::function<void(float)> &&callback) {
  ESP_LOGD(TAG, "Registering sensor callback for topic: %s", topic.c_str());
  if (!this->callback_manager_) {
    ESP_LOGE(TAG, "Cannot register sensor callback for %s: callback_manager is null", topic.c_str());
    return;
  }
  this->callback_manager_->register_sensor_callback(topic, std::move(callback));
  ESP_LOGCONFIG(TAG, "Sensor callback registered for topic: %s (total: %d)", 
                topic.c_str(), this->callback_manager_->get_sensor_callback_count());
}

void HeishamonComponent::decode_and_notify_sensors(const std::vector<uint8_t> &data) {
  ESP_LOGV(TAG, "Decoding sensor data from %d bytes", data.size());
  
  // Show registered callbacks count only at verbose level
  if (this->callback_manager_) {
    ESP_LOGV(TAG, "Callbacks: %d sensors, %d binary, %d switches, %d selects", 
             this->callback_manager_->get_sensor_callback_count(),
             this->callback_manager_->get_binary_sensor_callback_count(),
             this->callback_manager_->get_switch_callback_count(),
             this->callback_manager_->get_select_callback_count());
  }
  
  // Ensure we have enough data
  if (data.size() < 203) {
    ESP_LOGW(TAG, "Insufficient data size: %d, expected: 203", data.size());
    return;
  }
  
  // Decode ONLY topics that have callbacks registered (optimization)
  if (this->callback_manager_) {
    
    // DHW target temperature (byte 42: temperature - 128)
    if (this->callback_manager_->has_sensor_callback("dhw_target_temp")) {
      float dhw_target_temp = static_cast<float>(data[42] - 128);
      this->callback_manager_->notify_sensor_value("dhw_target_temp", dhw_target_temp);
    }
    
    // DHW temperature (byte 141: temperature - 128)
    if (this->callback_manager_->has_sensor_callback("dhw_temp")) {
      float dhw_temp = static_cast<float>(data[141] - 128);
      this->callback_manager_->notify_sensor_value("dhw_temp", dhw_temp);
    }
    
    // Outside temperature (byte 142: temperature - 128)  
    if (this->callback_manager_->has_sensor_callback("outside_temp")) {
      float outside_temp = static_cast<float>(data[142] - 128);
      this->callback_manager_->notify_sensor_value("outside_temp", outside_temp);
    }
    
    // Main inlet temperature (byte 143: temperature - 128, fractional in byte 118 bits 0-2)
    if (this->callback_manager_->has_sensor_callback("main_inlet_temp")) {
      float main_inlet_temp = static_cast<float>(data[143] - 128);
      int frac_inlet = data[118] & 0b111;
      if (frac_inlet == 2) main_inlet_temp += 0.25f;
      else if (frac_inlet == 3) main_inlet_temp += 0.50f;
      else if (frac_inlet == 4) main_inlet_temp += 0.75f;
      this->callback_manager_->notify_sensor_value("main_inlet_temp", main_inlet_temp);
    }
    
    // Main outlet temperature (byte 144: temperature - 128, fractional in byte 118 bits 3-5)
    if (this->callback_manager_->has_sensor_callback("main_outlet_temp")) {
      float main_outlet_temp = static_cast<float>(data[144] - 128);
      int frac_outlet = (data[118] >> 3) & 0b111;
      if (frac_outlet == 2) main_outlet_temp += 0.25f;
      else if (frac_outlet == 3) main_outlet_temp += 0.50f;
      else if (frac_outlet == 4) main_outlet_temp += 0.75f;
      this->callback_manager_->notify_sensor_value("main_outlet_temp", main_outlet_temp);
    }
    
    // Main target temperature (byte 153: temperature - 128)
    if (this->callback_manager_->has_sensor_callback("main_target_temp")) {
      float main_target_temp = static_cast<float>(data[153] - 128);
      this->callback_manager_->notify_sensor_value("main_target_temp", main_target_temp);
    }
    
    // Compressor frequency (byte 166: frequency - 1)
    if (this->callback_manager_->has_sensor_callback("compressor_freq")) {
      float compressor_freq = static_cast<float>(data[166] - 1);
      this->callback_manager_->notify_sensor_value("compressor_freq", compressor_freq);
    }
    
    // Operation mode sensor (byte 6: complex mode decoding)
    if (this->callback_manager_->has_sensor_callback("operation_mode")) {
      uint8_t mode = data[6] & 0b111111;
      float operation_mode = -1.0f;
      switch (mode) {
        case 18: operation_mode = 0.0f; break;
        case 19: operation_mode = 1.0f; break;
        case 25: operation_mode = 2.0f; break;
        case 33: operation_mode = 3.0f; break;
        case 34: operation_mode = 4.0f; break;
        case 35: operation_mode = 5.0f; break;
        case 41: operation_mode = 6.0f; break;
        case 26: operation_mode = 7.0f; break;
        case 42: operation_mode = 8.0f; break;
      }
      this->callback_manager_->notify_sensor_value("operation_mode", operation_mode);
    }
    
    // Pump flow (bytes 169-170: TOP1)
    // data[170] = integer part, data[169] = fractional part (value-1)/256
    if (this->callback_manager_->has_sensor_callback("pump_flow")) {
      float pump_flow = static_cast<float>(data[170]) + (static_cast<float>(static_cast<int>(data[169]) - 1)) / 256.0f;
      if (pump_flow < 0) pump_flow = 0;  // Handle edge case when data[169] is 0
      this->callback_manager_->notify_sensor_value("pump_flow", pump_flow);
    }

    // === ZONE TEMPERATURE TARGETS (for number entities) ===
    
    // Zone 1 Heat Request Temperature (byte 38: temperature - 128) - TOP27
    if (this->callback_manager_->has_sensor_callback("z1_heat_request_temp")) {
      float z1_heat_temp = static_cast<float>(data[38] - 128);
      this->callback_manager_->notify_sensor_value("z1_heat_request_temp", z1_heat_temp);
    }
    
    // Zone 1 Cool Request Temperature (byte 39: temperature - 128) - TOP28
    if (this->callback_manager_->has_sensor_callback("z1_cool_request_temp")) {
      float z1_cool_temp = static_cast<float>(data[39] - 128);
      this->callback_manager_->notify_sensor_value("z1_cool_request_temp", z1_cool_temp);
    }
    
    // Zone 2 Heat Request Temperature (byte 40: temperature - 128) - TOP34
    if (this->callback_manager_->has_sensor_callback("z2_heat_request_temp")) {
      float z2_heat_temp = static_cast<float>(data[40] - 128);
      this->callback_manager_->notify_sensor_value("z2_heat_request_temp", z2_heat_temp);
    }
    
    // Zone 2 Cool Request Temperature (byte 41: temperature - 128) - TOP35
    if (this->callback_manager_->has_sensor_callback("z2_cool_request_temp")) {
      float z2_cool_temp = static_cast<float>(data[41] - 128);
      this->callback_manager_->notify_sensor_value("z2_cool_request_temp", z2_cool_temp);
    }
    
    // Zone 1 Water Temperature (byte 145: temperature - 128) - TOP36
    if (this->callback_manager_->has_sensor_callback("z1_water_temp")) {
      float z1_water_temp = static_cast<float>(data[145] - 128);
      this->callback_manager_->notify_sensor_value("z1_water_temp", z1_water_temp);
    }
    
    // Zone 2 Water Temperature (byte 146: temperature - 128) - TOP37
    if (this->callback_manager_->has_sensor_callback("z2_water_temp")) {
      float z2_water_temp = static_cast<float>(data[146] - 128);
      this->callback_manager_->notify_sensor_value("z2_water_temp", z2_water_temp);
    }
    
    // Zone 1 Water Target Temperature (byte 147: temperature - 128) - TOP42
    if (this->callback_manager_->has_sensor_callback("z1_water_target_temp")) {
      float z1_water_target = static_cast<float>(data[147] - 128);
      this->callback_manager_->notify_sensor_value("z1_water_target_temp", z1_water_target);
    }
    
    // Zone 2 Water Target Temperature (byte 148: temperature - 128) - TOP43
    if (this->callback_manager_->has_sensor_callback("z2_water_target_temp")) {
      float z2_water_target = static_cast<float>(data[148] - 128);
      this->callback_manager_->notify_sensor_value("z2_water_target_temp", z2_water_target);
    }
    
    // Zone 1 Temperature (byte 139: temperature - 128) - TOP56
    if (this->callback_manager_->has_sensor_callback("z1_temp")) {
      float z1_temp = static_cast<float>(data[139] - 128);
      this->callback_manager_->notify_sensor_value("z1_temp", z1_temp);
    }
    
    // Zone 2 Temperature (byte 140: temperature - 128) - TOP57
    if (this->callback_manager_->has_sensor_callback("z2_temp")) {
      float z2_temp = static_cast<float>(data[140] - 128);
      this->callback_manager_->notify_sensor_value("z2_temp", z2_temp);
    }
    
    // === ADDITIONAL TEMPERATURES ===
    
    // Outside Pipe Temperature (byte 158: temperature - 128) - TOP21
    if (this->callback_manager_->has_sensor_callback("outside_pipe_temp")) {
      float outside_pipe_temp = static_cast<float>(data[158] - 128);
      this->callback_manager_->notify_sensor_value("outside_pipe_temp", outside_pipe_temp);
    }
    
    // Room Thermostat Temperature (byte 156: temperature - 128) - TOP33
    if (this->callback_manager_->has_sensor_callback("room_thermostat_temp")) {
      float room_thermo_temp = static_cast<float>(data[156] - 128);
      this->callback_manager_->notify_sensor_value("room_thermostat_temp", room_thermo_temp);
    }
    
    // Buffer Temperature (byte 149: temperature - 128) - TOP46
    if (this->callback_manager_->has_sensor_callback("buffer_temp")) {
      float buffer_temp = static_cast<float>(data[149] - 128);
      this->callback_manager_->notify_sensor_value("buffer_temp", buffer_temp);
    }
    
    // Solar Temperature (byte 150: temperature - 128) - TOP47
    if (this->callback_manager_->has_sensor_callback("solar_temp")) {
      float solar_temp = static_cast<float>(data[150] - 128);
      this->callback_manager_->notify_sensor_value("solar_temp", solar_temp);
    }
    
    // Pool Temperature (byte 151: temperature - 128) - TOP48
    if (this->callback_manager_->has_sensor_callback("pool_temp")) {
      float pool_temp = static_cast<float>(data[151] - 128);
      this->callback_manager_->notify_sensor_value("pool_temp", pool_temp);
    }
    
    // Main Hex Outlet Temperature (byte 154: temperature - 128) - TOP49
    if (this->callback_manager_->has_sensor_callback("main_hex_outlet_temp")) {
      float main_hex_outlet = static_cast<float>(data[154] - 128);
      this->callback_manager_->notify_sensor_value("main_hex_outlet_temp", main_hex_outlet);
    }
    
    // Discharge Temperature (byte 155: temperature - 128) - TOP50
    if (this->callback_manager_->has_sensor_callback("discharge_temp")) {
      float discharge_temp = static_cast<float>(data[155] - 128);
      this->callback_manager_->notify_sensor_value("discharge_temp", discharge_temp);
    }
    
    // Inside Pipe Temperature (byte 157: temperature - 128) - TOP51
    if (this->callback_manager_->has_sensor_callback("inside_pipe_temp")) {
      float inside_pipe_temp = static_cast<float>(data[157] - 128);
      this->callback_manager_->notify_sensor_value("inside_pipe_temp", inside_pipe_temp);
    }
    
    // Defrost Temperature (byte 159: temperature - 128) - TOP52
    if (this->callback_manager_->has_sensor_callback("defrost_temp")) {
      float defrost_temp = static_cast<float>(data[159] - 128);
      this->callback_manager_->notify_sensor_value("defrost_temp", defrost_temp);
    }
    
    // Eva Outlet Temperature (byte 160: temperature - 128) - TOP53
    if (this->callback_manager_->has_sensor_callback("eva_outlet_temp")) {
      float eva_outlet_temp = static_cast<float>(data[160] - 128);
      this->callback_manager_->notify_sensor_value("eva_outlet_temp", eva_outlet_temp);
    }
    
    // Bypass Outlet Temperature (byte 161: temperature - 128) - TOP54
    if (this->callback_manager_->has_sensor_callback("bypass_outlet_temp")) {
      float bypass_outlet_temp = static_cast<float>(data[161] - 128);
      this->callback_manager_->notify_sensor_value("bypass_outlet_temp", bypass_outlet_temp);
    }
    
    // IPM Temperature (byte 162: temperature - 128) - TOP55
    if (this->callback_manager_->has_sensor_callback("ipm_temp")) {
      float ipm_temp = static_cast<float>(data[162] - 128);
      this->callback_manager_->notify_sensor_value("ipm_temp", ipm_temp);
    }
    
    // Second Inlet Temperature (byte 126: temperature - 128) - TOP116
    if (this->callback_manager_->has_sensor_callback("second_inlet_temp")) {
      float second_inlet_temp = static_cast<float>(data[126] - 128);
      this->callback_manager_->notify_sensor_value("second_inlet_temp", second_inlet_temp);
    }
    
    // Economizer Outlet Temperature (byte 127: temperature - 128) - TOP117
    if (this->callback_manager_->has_sensor_callback("economizer_outlet_temp")) {
      float economizer_outlet_temp = static_cast<float>(data[127] - 128);
      this->callback_manager_->notify_sensor_value("economizer_outlet_temp", economizer_outlet_temp);
    }
    
    // Second Room Thermostat Temperature (byte 128: temperature - 128) - TOP118
    if (this->callback_manager_->has_sensor_callback("second_room_thermostat_temp")) {
      float second_room_thermo_temp = static_cast<float>(data[128] - 128);
      this->callback_manager_->notify_sensor_value("second_room_thermostat_temp", second_room_thermo_temp);
    }
    
    // === POWER SENSORS (uint16 values) ===
    
    // Heat Power Production (bytes 194-195) - TOP15
    if (this->callback_manager_->has_sensor_callback("heat_power_production")) {
      float heat_power_prod = static_cast<float>((data[194] - 1) * 200);
      this->callback_manager_->notify_sensor_value("heat_power_production", heat_power_prod);
    }
    
    // Heat Power Consumption (bytes 193-194) - TOP16
    if (this->callback_manager_->has_sensor_callback("heat_power_consumption")) {
      float heat_power_cons = static_cast<float>((data[193] - 1) * 200);
      this->callback_manager_->notify_sensor_value("heat_power_consumption", heat_power_cons);
    }
    
    // Cool Power Production (byte 196) - TOP38
    if (this->callback_manager_->has_sensor_callback("cool_power_production")) {
      float cool_power_prod = static_cast<float>((data[196] - 1) * 200);
      this->callback_manager_->notify_sensor_value("cool_power_production", cool_power_prod);
    }
    
    // Cool Power Consumption (byte 195) - TOP39
    if (this->callback_manager_->has_sensor_callback("cool_power_consumption")) {
      float cool_power_cons = static_cast<float>((data[195] - 1) * 200);
      this->callback_manager_->notify_sensor_value("cool_power_consumption", cool_power_cons);
    }
    
    // DHW Power Production (byte 198) - TOP40
    if (this->callback_manager_->has_sensor_callback("dhw_power_production")) {
      float dhw_power_prod = static_cast<float>((data[198] - 1) * 200);
      this->callback_manager_->notify_sensor_value("dhw_power_production", dhw_power_prod);
    }
    
    // DHW Power Consumption (byte 197) - TOP41
    if (this->callback_manager_->has_sensor_callback("dhw_power_consumption")) {
      float dhw_power_cons = static_cast<float>((data[197] - 1) * 200);
      this->callback_manager_->notify_sensor_value("dhw_power_consumption", dhw_power_cons);
    }
    
    // === FAN AND PRESSURE SENSORS ===
    
    // Fan1 Motor Speed (byte 173: (value - 1) * 10) - TOP62
    if (this->callback_manager_->has_sensor_callback("fan1_motor_speed")) {
      float fan1_speed = static_cast<float>((data[173] - 1) * 10);
      this->callback_manager_->notify_sensor_value("fan1_motor_speed", fan1_speed);
    }
    
    // Fan2 Motor Speed (byte 174: (value - 1) * 10) - TOP63
    if (this->callback_manager_->has_sensor_callback("fan2_motor_speed")) {
      float fan2_speed = static_cast<float>((data[174] - 1) * 10);
      this->callback_manager_->notify_sensor_value("fan2_motor_speed", fan2_speed);
    }
    
    // High Pressure (byte 163: (value - 1) / 5) - TOP64
    if (this->callback_manager_->has_sensor_callback("high_pressure")) {
      float high_pressure = static_cast<float>(data[163] - 1) / 5.0f;
      this->callback_manager_->notify_sensor_value("high_pressure", high_pressure);
    }
    
    // Low Pressure (byte 164: (value - 1) * 50) - TOP66
    if (this->callback_manager_->has_sensor_callback("low_pressure")) {
      float low_pressure = static_cast<float>((data[164] - 1) * 50);
      this->callback_manager_->notify_sensor_value("low_pressure", low_pressure);
    }
    
    // Pump Speed (byte 171: (value - 1) * 50) - TOP65
    if (this->callback_manager_->has_sensor_callback("pump_speed")) {
      float pump_speed = static_cast<float>((data[171] - 1) * 50);
      this->callback_manager_->notify_sensor_value("pump_speed", pump_speed);
    }
    
    // Compressor Current (byte 165: (value - 1) / 5) - TOP67
    if (this->callback_manager_->has_sensor_callback("compressor_current")) {
      float compressor_current = static_cast<float>(data[165] - 1) / 5.0f;
      this->callback_manager_->notify_sensor_value("compressor_current", compressor_current);
    }
    
    // Pump Duty (byte 172: value - 1) - TOP93
    if (this->callback_manager_->has_sensor_callback("pump_duty")) {
      float pump_duty = static_cast<float>(data[172] - 1);
      this->callback_manager_->notify_sensor_value("pump_duty", pump_duty);
    }
    
    // Max Pump Duty (byte 45: value - 1) - TOP95
    if (this->callback_manager_->has_sensor_callback("max_pump_duty")) {
      float max_pump_duty = static_cast<float>(data[45] - 1);
      this->callback_manager_->notify_sensor_value("max_pump_duty", max_pump_duty);
    }
    
    // Water Pressure (byte 125: (value - 1) / 50) - TOP115
    if (this->callback_manager_->has_sensor_callback("water_pressure")) {
      float water_pressure = static_cast<float>(data[125] - 1) / 50.0f;
      this->callback_manager_->notify_sensor_value("water_pressure", water_pressure);
    }
    
    // Zone 1 Valve PID (byte 177: (value - 1) / 2) - TOP127
    if (this->callback_manager_->has_sensor_callback("z1_valve_pid")) {
      float z1_valve_pid = static_cast<float>(data[177] - 1) / 2.0f;
      this->callback_manager_->notify_sensor_value("z1_valve_pid", z1_valve_pid);
    }
    
    // Zone 2 Valve PID (byte 178: (value - 1) / 2) - TOP128
    if (this->callback_manager_->has_sensor_callback("z2_valve_pid")) {
      float z2_valve_pid = static_cast<float>(data[178] - 1) / 2.0f;
      this->callback_manager_->notify_sensor_value("z2_valve_pid", z2_valve_pid);
    }
    
    // === BIVALENT SENSORS ===
    
    // Bivalent Start Temp (byte 65: value - 128) - TOP131
    if (this->callback_manager_->has_sensor_callback("bivalent_start_temp")) {
      float bivalent_start_temp = static_cast<float>(static_cast<int8_t>(data[65] - 128));
      this->callback_manager_->notify_sensor_value("bivalent_start_temp", bivalent_start_temp);
    }
    
    // Bivalent Advanced Start Temp (byte 66: value - 128) - TOP134
    if (this->callback_manager_->has_sensor_callback("bivalent_advanced_start_temp")) {
      float bivalent_ap_start_temp = static_cast<float>(static_cast<int8_t>(data[66] - 128));
      this->callback_manager_->notify_sensor_value("bivalent_advanced_start_temp", bivalent_ap_start_temp);
    }
    
    // Bivalent Advanced Stop Temp (byte 68: value - 128) - TOP135
    if (this->callback_manager_->has_sensor_callback("bivalent_advanced_stop_temp")) {
      float bivalent_ap_stop_temp = static_cast<float>(static_cast<int8_t>(data[68] - 128));
      this->callback_manager_->notify_sensor_value("bivalent_advanced_stop_temp", bivalent_ap_stop_temp);
    }
    
    // Bivalent Advanced Start Delay (byte 67: value - 1) - TOP136
    if (this->callback_manager_->has_sensor_callback("bivalent_advanced_start_delay")) {
      float bivalent_start_delay = static_cast<float>(data[67] - 1);
      this->callback_manager_->notify_sensor_value("bivalent_advanced_start_delay", bivalent_start_delay);
    }
    
    // Bivalent Advanced Stop Delay (byte 69: value - 1) - TOP137
    if (this->callback_manager_->has_sensor_callback("bivalent_advanced_stop_delay")) {
      float bivalent_stop_delay = static_cast<float>(data[69] - 1);
      this->callback_manager_->notify_sensor_value("bivalent_advanced_stop_delay", bivalent_stop_delay);
    }
    
    // Bivalent Advanced DHW Delay (byte 70: value - 1) - TOP138
    if (this->callback_manager_->has_sensor_callback("bivalent_advanced_dhw_delay")) {
      float bivalent_dhw_delay = static_cast<float>(data[70] - 1);
      this->callback_manager_->notify_sensor_value("bivalent_advanced_dhw_delay", bivalent_dhw_delay);
    }
    
    // === AUTO MODE OUTDOOR TEMPERATURES ===
    
    // Heating Off Outdoor Temp (byte 83: value - 128) - TOP77
    if (this->callback_manager_->has_sensor_callback("heating_off_outdoor_temp")) {
      float heating_off_outdoor_temp = static_cast<float>(static_cast<int8_t>(data[83] - 128));
      this->callback_manager_->notify_sensor_value("heating_off_outdoor_temp", heating_off_outdoor_temp);
    }
    
    // Heater On Outdoor Temp (byte 85: value - 128) - TOP78
    if (this->callback_manager_->has_sensor_callback("heater_on_outdoor_temp")) {
      float heater_on_outdoor_temp = static_cast<float>(static_cast<int8_t>(data[85] - 128));
      this->callback_manager_->notify_sensor_value("heater_on_outdoor_temp", heater_on_outdoor_temp);
    }
    
    // Heat to Cool Temp (byte 95: value - 128) - TOP79
    if (this->callback_manager_->has_sensor_callback("heat_to_cool_temp")) {
      float heat_to_cool_temp = static_cast<float>(static_cast<int8_t>(data[95] - 128));
      this->callback_manager_->notify_sensor_value("heat_to_cool_temp", heat_to_cool_temp);
    }
    
    // Cool to Heat Temp (byte 96: value - 128) - TOP80
    if (this->callback_manager_->has_sensor_callback("cool_to_heat_temp")) {
      float cool_to_heat_temp = static_cast<float>(static_cast<int8_t>(data[96] - 128));
      this->callback_manager_->notify_sensor_value("cool_to_heat_temp", cool_to_heat_temp);
    }
    
    // === J-SERIES HEATER SENSORS ===
    
    // Heater Delay Time (byte 104: value - 1) - TOP96
    if (this->callback_manager_->has_sensor_callback("heater_delay_time")) {
      float heater_delay_time = static_cast<float>(data[104]) - 1.0f;
      this->callback_manager_->notify_sensor_value("heater_delay_time", heater_delay_time);
    }
    
    // Heater Start Delta (byte 105: value - 128) - TOP97
    if (this->callback_manager_->has_sensor_callback("heater_start_delta")) {
      float heater_start_delta = static_cast<float>(static_cast<int8_t>(data[105] - 128));
      this->callback_manager_->notify_sensor_value("heater_start_delta", heater_start_delta);
    }
    
    // Heater Stop Delta (byte 106: value - 128) - TOP98
    if (this->callback_manager_->has_sensor_callback("heater_stop_delta")) {
      float heater_stop_delta = static_cast<float>(static_cast<int8_t>(data[106] - 128));
      this->callback_manager_->notify_sensor_value("heater_stop_delta", heater_stop_delta);
    }
    
    // === HOLIDAY SHIFT TEMPERATURES ===
    
    // DHW Holiday Shift Temp (byte 44: value - 128) - TOP25
    if (this->callback_manager_->has_sensor_callback("dhw_holiday_shift_temp")) {
      float dhw_holiday_shift = static_cast<float>(static_cast<int8_t>(data[44] - 128));
      this->callback_manager_->notify_sensor_value("dhw_holiday_shift_temp", dhw_holiday_shift);
    }
    
    // Room Holiday Shift Temp (byte 43: value - 128) - TOP45
    if (this->callback_manager_->has_sensor_callback("room_holiday_shift_temp")) {
      float room_holiday_shift = static_cast<float>(static_cast<int8_t>(data[43] - 128));
      this->callback_manager_->notify_sensor_value("room_holiday_shift_temp", room_holiday_shift);
    }
    
    // === SOLAR SENSORS ===
    
    // Solar On Delta (byte 61: value - 128) - TOP102
    if (this->callback_manager_->has_sensor_callback("solar_on_delta")) {
      float solar_on_delta = static_cast<float>(static_cast<int8_t>(data[61] - 128));
      this->callback_manager_->notify_sensor_value("solar_on_delta", solar_on_delta);
    }
    
    // Solar Off Delta (byte 62: value - 128) - TOP103
    if (this->callback_manager_->has_sensor_callback("solar_off_delta")) {
      float solar_off_delta = static_cast<float>(static_cast<int8_t>(data[62] - 128));
      this->callback_manager_->notify_sensor_value("solar_off_delta", solar_off_delta);
    }
    
    // Solar Frost Protection (byte 63: value - 128) - TOP104
    if (this->callback_manager_->has_sensor_callback("solar_frost_protection")) {
      float solar_frost = static_cast<float>(static_cast<int8_t>(data[63] - 128));
      this->callback_manager_->notify_sensor_value("solar_frost_protection", solar_frost);
    }
    
    // Solar High Limit (byte 64: value - 128) - TOP105
    if (this->callback_manager_->has_sensor_callback("solar_high_limit")) {
      float solar_high_limit = static_cast<float>(static_cast<int8_t>(data[64] - 128));
      this->callback_manager_->notify_sensor_value("solar_high_limit", solar_high_limit);
    }
    
    // Buffer Tank Delta (byte 59: value - 128) - TOP113
    if (this->callback_manager_->has_sensor_callback("buffer_tank_delta")) {
      float buffer_tank_delta = static_cast<float>(static_cast<int8_t>(data[59] - 128));
      this->callback_manager_->notify_sensor_value("buffer_tank_delta", buffer_tank_delta);
    }
    
    // === ZONE 1 HEAT CURVE ===
    
    // Z1 Heat Curve Target High (byte 75: value - 128) - TOP29
    if (this->callback_manager_->has_sensor_callback("z1_heat_curve_target_high")) {
      float value = static_cast<float>(static_cast<int8_t>(data[75] - 128));
      this->callback_manager_->notify_sensor_value("z1_heat_curve_target_high", value);
    }
    
    // Z1 Heat Curve Target Low (byte 76: value - 128) - TOP30
    if (this->callback_manager_->has_sensor_callback("z1_heat_curve_target_low")) {
      float value = static_cast<float>(static_cast<int8_t>(data[76] - 128));
      this->callback_manager_->notify_sensor_value("z1_heat_curve_target_low", value);
    }
    
    // Z1 Heat Curve Outside High (byte 78: value - 128) - TOP31
    if (this->callback_manager_->has_sensor_callback("z1_heat_curve_outside_high")) {
      float value = static_cast<float>(static_cast<int8_t>(data[78] - 128));
      this->callback_manager_->notify_sensor_value("z1_heat_curve_outside_high", value);
    }
    
    // Z1 Heat Curve Outside Low (byte 77: value - 128) - TOP32
    if (this->callback_manager_->has_sensor_callback("z1_heat_curve_outside_low")) {
      float value = static_cast<float>(static_cast<int8_t>(data[77] - 128));
      this->callback_manager_->notify_sensor_value("z1_heat_curve_outside_low", value);
    }
    
    // === ZONE 1 COOL CURVE ===
    
    // Z1 Cool Curve Target High (byte 86: value - 128) - TOP72
    if (this->callback_manager_->has_sensor_callback("z1_cool_curve_target_high")) {
      float value = static_cast<float>(static_cast<int8_t>(data[86] - 128));
      this->callback_manager_->notify_sensor_value("z1_cool_curve_target_high", value);
    }
    
    // Z1 Cool Curve Target Low (byte 87: value - 128) - TOP73
    if (this->callback_manager_->has_sensor_callback("z1_cool_curve_target_low")) {
      float value = static_cast<float>(static_cast<int8_t>(data[87] - 128));
      this->callback_manager_->notify_sensor_value("z1_cool_curve_target_low", value);
    }
    
    // Z1 Cool Curve Outside High (byte 89: value - 128) - TOP74
    if (this->callback_manager_->has_sensor_callback("z1_cool_curve_outside_high")) {
      float value = static_cast<float>(static_cast<int8_t>(data[89] - 128));
      this->callback_manager_->notify_sensor_value("z1_cool_curve_outside_high", value);
    }
    
    // Z1 Cool Curve Outside Low (byte 88: value - 128) - TOP75
    if (this->callback_manager_->has_sensor_callback("z1_cool_curve_outside_low")) {
      float value = static_cast<float>(static_cast<int8_t>(data[88] - 128));
      this->callback_manager_->notify_sensor_value("z1_cool_curve_outside_low", value);
    }
    
    // === ZONE 2 HEAT CURVE ===
    
    // Z2 Heat Curve Target High (byte 79: value - 128) - TOP82
    if (this->callback_manager_->has_sensor_callback("z2_heat_curve_target_high")) {
      float value = static_cast<float>(static_cast<int8_t>(data[79] - 128));
      this->callback_manager_->notify_sensor_value("z2_heat_curve_target_high", value);
    }
    
    // Z2 Heat Curve Target Low (byte 80: value - 128) - TOP83
    if (this->callback_manager_->has_sensor_callback("z2_heat_curve_target_low")) {
      float value = static_cast<float>(static_cast<int8_t>(data[80] - 128));
      this->callback_manager_->notify_sensor_value("z2_heat_curve_target_low", value);
    }
    
    // Z2 Heat Curve Outside High (byte 82: value - 128) - TOP84
    if (this->callback_manager_->has_sensor_callback("z2_heat_curve_outside_high")) {
      float value = static_cast<float>(static_cast<int8_t>(data[82] - 128));
      this->callback_manager_->notify_sensor_value("z2_heat_curve_outside_high", value);
    }
    
    // Z2 Heat Curve Outside Low (byte 81: value - 128) - TOP85
    if (this->callback_manager_->has_sensor_callback("z2_heat_curve_outside_low")) {
      float value = static_cast<float>(static_cast<int8_t>(data[81] - 128));
      this->callback_manager_->notify_sensor_value("z2_heat_curve_outside_low", value);
    }
    
    // === ZONE 2 COOL CURVE ===
    
    // Z2 Cool Curve Target High (byte 90: value - 128) - TOP86
    if (this->callback_manager_->has_sensor_callback("z2_cool_curve_target_high")) {
      float value = static_cast<float>(static_cast<int8_t>(data[90] - 128));
      this->callback_manager_->notify_sensor_value("z2_cool_curve_target_high", value);
    }
    
    // Z2 Cool Curve Target Low (byte 91: value - 128) - TOP87
    if (this->callback_manager_->has_sensor_callback("z2_cool_curve_target_low")) {
      float value = static_cast<float>(static_cast<int8_t>(data[91] - 128));
      this->callback_manager_->notify_sensor_value("z2_cool_curve_target_low", value);
    }
    
    // Z2 Cool Curve Outside High (byte 93: value - 128) - TOP88
    if (this->callback_manager_->has_sensor_callback("z2_cool_curve_outside_high")) {
      float value = static_cast<float>(static_cast<int8_t>(data[93] - 128));
      this->callback_manager_->notify_sensor_value("z2_cool_curve_outside_high", value);
    }
    
    // Z2 Cool Curve Outside Low (byte 92: value - 128) - TOP89
    if (this->callback_manager_->has_sensor_callback("z2_cool_curve_outside_low")) {
      float value = static_cast<float>(static_cast<int8_t>(data[92] - 128));
      this->callback_manager_->notify_sensor_value("z2_cool_curve_outside_low", value);
    }
    
    // === STERILIZATION ===
    
    // Sterilization Temp (byte 100: value - 128) - TOP70
    if (this->callback_manager_->has_sensor_callback("sterilization_temp")) {
      float sterilization_temp = static_cast<float>(static_cast<int8_t>(data[100] - 128));
      this->callback_manager_->notify_sensor_value("sterilization_temp", sterilization_temp);
    }
    
    // Sterilization Max Time (byte 101: value - 1) - TOP71
    if (this->callback_manager_->has_sensor_callback("sterilization_max_time")) {
      float sterilization_max_time = static_cast<float>(data[101] - 1);
      this->callback_manager_->notify_sensor_value("sterilization_max_time", sterilization_max_time);
    }
    
    // === DELTA TEMPERATURES ===
    
    // DHW Heat Delta (byte 99: temperature - 128) - TOP22
    if (this->callback_manager_->has_sensor_callback("dhw_heat_delta")) {
      float dhw_heat_delta = static_cast<float>(data[99] - 128);
      this->callback_manager_->notify_sensor_value("dhw_heat_delta", dhw_heat_delta);
    }
    
    // Heat Delta (byte 84: temperature - 128) - TOP23
    if (this->callback_manager_->has_sensor_callback("heat_delta")) {
      float heat_delta = static_cast<float>(data[84] - 128);
      this->callback_manager_->notify_sensor_value("heat_delta", heat_delta);
    }
    
    // Cool Delta (byte 94: temperature - 128) - TOP24
    if (this->callback_manager_->has_sensor_callback("cool_delta")) {
      float cool_delta = static_cast<float>(data[94] - 128);
      this->callback_manager_->notify_sensor_value("cool_delta", cool_delta);
    }

    // === OPERATIONS COUNTERS (uint16 values) ===
    
    // Operations Hours (bytes 182-183: word - 1) - TOP11
    if (this->callback_manager_->has_sensor_callback("operations_hours")) {
      uint16_t ops_hours = (static_cast<uint16_t>(data[183]) << 8) | data[182];
      float operations_hours = static_cast<float>(ops_hours - 1);
      this->callback_manager_->notify_sensor_value("operations_hours", operations_hours);
    }
    
    // Operations Counter (bytes 179-180: word - 1) - TOP12
    if (this->callback_manager_->has_sensor_callback("operations_counter")) {
      uint16_t ops_counter = (static_cast<uint16_t>(data[180]) << 8) | data[179];
      float operations_counter = static_cast<float>(ops_counter - 1);
      this->callback_manager_->notify_sensor_value("operations_counter", operations_counter);
    }
    
    // Room Heater Operations Hours (bytes 185-186: word - 1) - TOP90
    if (this->callback_manager_->has_sensor_callback("room_heater_operations_hours")) {
      uint16_t room_heater_hours = (static_cast<uint16_t>(data[186]) << 8) | data[185];
      float room_heater_ops_hours = static_cast<float>(room_heater_hours - 1);
      this->callback_manager_->notify_sensor_value("room_heater_operations_hours", room_heater_ops_hours);
    }
    
    // DHW Heater Operations Hours (bytes 188-189: word - 1) - TOP91
    if (this->callback_manager_->has_sensor_callback("dhw_heater_operations_hours")) {
      uint16_t dhw_heater_hours = (static_cast<uint16_t>(data[189]) << 8) | data[188];
      float dhw_heater_ops_hours = static_cast<float>(dhw_heater_hours - 1);
      this->callback_manager_->notify_sensor_value("dhw_heater_operations_hours", dhw_heater_ops_hours);
    }

    // === BINARY SENSORS ===
    
    // Heat pump state (byte 4, bits 7&8 in Panasonic = bits 0&1) - TOP0
    // getBit7and8: (input & 0b11) - 1, so 0b01=0(off), 0b10=1(on)
    if (this->callback_manager_->has_binary_sensor_callback("heatpump_state")) {
      int heatpump_value = data[4] & 0b11;
      bool heatpump_running = (heatpump_value == 0b10);
      this->callback_manager_->notify_binary_sensor_value("heatpump_state", heatpump_running);
    }
    
    // Force DHW State (byte 4, bit 7) - TOP2
    // 0x56 (86) = off, 0x96 (150) = on - difference is bit 7 (0x80)
    if (this->callback_manager_->has_binary_sensor_callback("force_dhw_state")) {
      bool force_dhw_active = (data[4] & 0x80) != 0;  // bit 7 (0x80 = 0b10000000)
      this->callback_manager_->notify_binary_sensor_value("force_dhw_state", force_dhw_active);
    }
    
    // Defrosting State (byte 111, bits 3&4 from right) - TOP26
    // 0b01 = not active, 0b10 = active
    if (this->callback_manager_->has_binary_sensor_callback("defrosting_state")) {
      int defrost_value = (data[111] >> 2) & 0b11;
      bool defrosting = (defrost_value == 0b10);
      this->callback_manager_->notify_binary_sensor_value("defrosting_state", defrosting);
    }
    
    // Internal Heater State (byte 112, bits 7&8) - TOP60
    if (this->callback_manager_->has_binary_sensor_callback("internal_heater_state")) {
      int internal_heater_value = (data[112] >> 6) - 1;
      bool internal_heater = (internal_heater_value > 0);
      this->callback_manager_->notify_binary_sensor_value("internal_heater_state", internal_heater);
    }
    
    // External Heater State (byte 112, bits 5&6) - TOP61
    if (this->callback_manager_->has_binary_sensor_callback("external_heater_state")) {
      int external_heater_value = ((data[112] >> 4) & 0b11) - 1;
      bool external_heater = (external_heater_value > 0);
      this->callback_manager_->notify_binary_sensor_value("external_heater_state", external_heater);
    }
    
    // DHW Installed (byte 24, bits 7&8 in Panasonic numbering = bits 0&1) - TOP100
    // getBit7and8: (input & 0b11) - 1, so 0b01=0(disabled), 0b10=1(enabled)
    if (this->callback_manager_->has_binary_sensor_callback("dhw_installed")) {
      int dhw_installed_value = data[24] & 0b11;
      bool dhw_installed = (dhw_installed_value == 0b10);
      this->callback_manager_->notify_binary_sensor_value("dhw_installed", dhw_installed);
    }
    
    // Zone 1 Pump State (byte 116, getBit3and4) - TOP124
    // Original HeishaMon: getBit3and4 = ((input >> 4) & 0b11) - 1
    // Result: 0=OFF, 1=ON
    if (this->callback_manager_->has_binary_sensor_callback("z1_pump_state")) {
      uint8_t z1_pump_value = (data[116] >> 4) & 0b11;
      bool z1_pump = (z1_pump_value == 0b10);  // 0b10 - 1 = 1 = ON
      ESP_LOGV(TAG, "Z1 pump: byte116=0x%02X, getBit3and4=0x%02X, state=%s", data[116], z1_pump_value, z1_pump ? "ON" : "OFF");
      this->callback_manager_->notify_binary_sensor_value("z1_pump_state", z1_pump);
    }
    
    // Zone 2 Pump State (byte 116, getBit1and2) - TOP123
    // Original HeishaMon: getBit1and2 = ((input >> 6) - 1)
    // Result: 0=OFF, 1=ON
    if (this->callback_manager_->has_binary_sensor_callback("z2_pump_state")) {
      uint8_t z2_pump_value = (data[116] >> 6) & 0b11;
      bool z2_pump = (z2_pump_value == 0b10);  // 0b10 - 1 = 1 = ON
      ESP_LOGV(TAG, "Z2 pump: byte116=0x%02X, getBit1and2=0x%02X, state=%s", data[116], z2_pump_value, z2_pump ? "ON" : "OFF");
      this->callback_manager_->notify_binary_sensor_value("z2_pump_state", z2_pump);
    }
    
    // Alarm State (bytes 113-114, error decoding) - TOP44
    if (this->callback_manager_->has_binary_sensor_callback("alarm_state")) {
      int error_type = data[113];
      // Error types: 177 = F type, 161 = H type, otherwise no error
      bool alarm = (error_type == 177 || error_type == 161);
      this->callback_manager_->notify_binary_sensor_value("alarm_state", alarm);
    }
    
    // === TEXT SENSORS ===
    
    // Error Code (bytes 113-114, text decoding) - TOP44
    if (this->callback_manager_->has_text_sensor_callback("error")) {
      int error_type = data[113];
      int error_number = data[114] - 17;
      char error_string[16];
      switch (error_type) {
        case 177:  // 0xB1 = F type error
          snprintf(error_string, sizeof(error_string), "F%02X", error_number);
          break;
        case 161:  // 0xA1 = H type error
          snprintf(error_string, sizeof(error_string), "H%02X", error_number);
          break;
        default:
          snprintf(error_string, sizeof(error_string), "No error");
          break;
      }
      this->callback_manager_->notify_text_sensor_value("error", std::string(error_string));
    }
    
    // Heat Pump Model (bytes 129-138, hex string) - TOP92
    if (this->callback_manager_->has_text_sensor_callback("heat_pump_model")) {
      char model_string[32];
      snprintf(model_string, sizeof(model_string), "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
               data[129], data[130], data[131], data[132], data[133],
               data[134], data[135], data[136], data[137], data[138]);
      this->callback_manager_->notify_text_sensor_value("heat_pump_model", std::string(model_string));
    }
    
    // DHW heating active (byte 112, bits 1&2)
    if (this->callback_manager_->has_binary_sensor_callback("dhw_heating")) {
      bool dhw_heating = (data[112] & 0b11) > 0;
      this->callback_manager_->notify_binary_sensor_value("dhw_heating", dhw_heating);
    }

    // === SWITCHES ===
    
    // Force DHW Mode (byte 112, bits 7&8)
    if (this->callback_manager_->has_switch_callback("force_dhw")) {
      float force_dhw_value = ((data[112] >> 6) - 1);
      bool force_dhw = (force_dhw_value > 0);
      ESP_LOGV(TAG, "Force DHW Switch: byte=0x%02X, state=%s", data[112], force_dhw ? "ON" : "OFF");
      this->callback_manager_->notify_switch_value("force_dhw", force_dhw);
    }

    // Holiday mode (byte 113, bits 5&6)
    if (this->callback_manager_->has_switch_callback("holiday_mode")) {
      float holiday_mode_value = (((data[113] >> 2) & 0b11) - 1);
      bool holiday_mode = (holiday_mode_value > 0);
      ESP_LOGV(TAG, "Holiday Mode Switch: byte=0x%02X, state=%s", data[113], holiday_mode ? "ON" : "OFF");
      this->callback_manager_->notify_switch_value("holiday_mode", holiday_mode);
    }

    // === SELECTS ===
    
    // Operation Mode select (byte 6)
    if (this->callback_manager_->has_select_callback("operation_mode")) {
      std::string operation_mode_text = this->decode_operation_mode(data[6]);
      ESP_LOGV(TAG, "Operation Mode Select: byte=0x%02X, value=%s", data[6], operation_mode_text.c_str());
      this->callback_manager_->notify_select_value("operation_mode", operation_mode_text);
    }
  }
}

void HeishamonComponent::register_binary_sensor_callback(const std::string &topic, std::function<void(bool)> callback) {
  ESP_LOGD(TAG, "Registering binary sensor callback for topic: %s", topic.c_str());
  if (this->callback_manager_) {
    this->callback_manager_->register_binary_sensor_callback(topic, std::move(callback));
    ESP_LOGCONFIG(TAG, "Binary sensor callback registered for topic: %s (total: %d)", 
                  topic.c_str(), this->callback_manager_->get_binary_sensor_callback_count());
  } else {
    ESP_LOGE(TAG, "Cannot register binary sensor callback for %s: callback_manager is null", topic.c_str());
  }
}

void HeishamonComponent::register_switch_callback(const std::string &topic, std::function<void(bool)> callback) {
  ESP_LOGD(TAG, "Registering switch callback for topic: %s", topic.c_str());
  if (this->callback_manager_) {
    this->callback_manager_->register_switch_callback(topic, std::move(callback));
    ESP_LOGCONFIG(TAG, "Switch callback registered for topic: %s (total: %d)", 
                  topic.c_str(), this->callback_manager_->get_switch_callback_count());
  } else {
    ESP_LOGE(TAG, "Cannot register switch callback for %s: callback_manager is null", topic.c_str());
  }
}

void HeishamonComponent::register_select_callback(const std::string &topic, std::function<void(const std::string&)> callback) {
  ESP_LOGD(TAG, "Registering select callback for topic: %s", topic.c_str());
  if (this->callback_manager_) {
    this->callback_manager_->register_select_callback(topic, std::move(callback));
    ESP_LOGCONFIG(TAG, "Select callback registered for topic: %s (total: %d)", 
                  topic.c_str(), this->callback_manager_->get_select_callback_count());
  } else {
    ESP_LOGE(TAG, "Cannot register select callback for %s: callback_manager is null", topic.c_str());
  }
}

void HeishamonComponent::register_text_sensor_callback(const std::string &topic, std::function<void(const std::string&)> callback) {
  ESP_LOGD(TAG, "Registering text sensor callback for topic: %s", topic.c_str());
  if (this->callback_manager_) {
    this->callback_manager_->register_text_sensor_callback(topic, std::move(callback));
    ESP_LOGCONFIG(TAG, "Text sensor callback registered for topic: %s (total: %d)", 
                  topic.c_str(), this->callback_manager_->get_text_sensor_callback_count());
  } else {
    ESP_LOGE(TAG, "Cannot register text sensor callback for %s: callback_manager is null", topic.c_str());
  }
}

bool HeishamonComponent::send_command(const std::vector<uint8_t> &command) {
  if (this->protocol_) {
    return this->protocol_->send_command(command);
  }
  return false;
}

void HeishamonComponent::send_command(const std::string &command) {
  // Delegate to protocol layer or command manager when implemented
  ESP_LOGD(TAG, "Send command: %s", command.c_str());
}

void HeishamonComponent::send_command(const std::string &command, const std::string &value) {
  // Delegate to protocol layer or command manager when implemented  
  ESP_LOGD(TAG, "Send command: %s = %s", command.c_str(), value.c_str());
}

void HeishamonComponent::send_command(const std::string &command, uint8_t value) {
  // Create and send the command packet to the heat pump
  ESP_LOGD(TAG, "Send command: %s = %d", command.c_str(), value);
  this->create_command(command, value);
}

void HeishamonComponent::create_command(const std::string &command, uint8_t value) {
  // Delegate to protocol layer or command manager when implemented
  ESP_LOGD(TAG, "Create command: %s = %d", command.c_str(), value);
}

bool HeishamonComponent::send_number_command(const std::string &command, float value) {
  // Delegate to protocol layer or command manager when implemented
  ESP_LOGD(TAG, "Send number command: %s = %.1f", command.c_str(), value);
  return true;
}

// Climate control stubs - these will be implemented in Phase 2
void HeishamonComponent::set_heat_mode_enabled(bool enabled) {
  ESP_LOGD(TAG, "Set heat mode enabled: %s", YESNO(enabled));
}

void HeishamonComponent::set_cool_mode_enabled(bool enabled) {
  ESP_LOGD(TAG, "Set cool mode enabled: %s", YESNO(enabled));
}

void HeishamonComponent::set_zone1_heat_enabled(bool enabled) {
  ESP_LOGD(TAG, "Set zone1 heat enabled: %s", YESNO(enabled));
}

void HeishamonComponent::set_zone1_cool_enabled(bool enabled) {
  ESP_LOGD(TAG, "Set zone1 cool enabled: %s", YESNO(enabled));
}

void HeishamonComponent::set_zone2_heat_enabled(bool enabled) {
  ESP_LOGD(TAG, "Set zone2 heat enabled: %s", YESNO(enabled));
}

void HeishamonComponent::set_zone2_cool_enabled(bool enabled) {
  ESP_LOGD(TAG, "Set zone2 cool enabled: %s", YESNO(enabled));
}

void HeishamonComponent::set_zone1_heat_target_temperature(float temperature) {
  ESP_LOGD(TAG, "Set zone1 heat target temperature: %.1f", temperature);
}

void HeishamonComponent::set_zone1_cool_target_temperature(float temperature) {
  ESP_LOGD(TAG, "Set zone1 cool target temperature: %.1f", temperature);
}

void HeishamonComponent::set_zone2_heat_target_temperature(float temperature) {
  ESP_LOGD(TAG, "Set zone2 heat target temperature: %.1f", temperature);
}

void HeishamonComponent::set_zone2_cool_target_temperature(float temperature) {
  ESP_LOGD(TAG, "Set zone2 cool target temperature: %.1f", temperature);
}

float HeishamonComponent::get_zone1_current_temperature() const {
  return 20.0f; // Stub value
}

float HeishamonComponent::get_zone2_current_temperature() const {
  return 20.0f; // Stub value
}

float HeishamonComponent::get_zone1_heat_target_temperature() const {
  return 21.0f; // Stub value
}

float HeishamonComponent::get_zone1_cool_target_temperature() const {
  return 22.0f; // Stub value
}

float HeishamonComponent::get_zone2_heat_target_temperature() const {
  return 21.0f; // Stub value
}

float HeishamonComponent::get_zone2_cool_target_temperature() const {
  return 22.0f; // Stub value
}

bool HeishamonComponent::get_heat_mode_enabled() const {
  return false; // Stub value
}

bool HeishamonComponent::get_cool_mode_enabled() const {
  return false; // Stub value
}

bool HeishamonComponent::get_zone1_heat_enabled() const {
  return false; // Stub value
}

bool HeishamonComponent::get_zone1_cool_enabled() const {
  return false; // Stub value
}

bool HeishamonComponent::get_zone2_heat_enabled() const {
  return false; // Stub value
}

bool HeishamonComponent::get_zone2_cool_enabled() const {
  return false; // Stub value
}

#ifdef USE_WATER_HEATER
float HeishamonComponent::get_dhw_current_temperature() const {
  return 40.0f; // Stub value
}
#endif

// Select value decoding functions
std::string HeishamonComponent::decode_operation_mode(uint8_t input) {
  // Apply 6-bit mask as per HeishaMon protocol
  uint8_t mode = input & 0b111111;
  
  // Return strings matching select.py options exactly
  switch (mode) {
    case 18: return "Heat only";
    case 19: return "Cool only"; 
    case 25: return "Auto(Heat)";
    case 33: return "DHW only";
    case 34: return "Heat+DHW";
    case 35: return "Cool+DHW";
    case 41: return "Auto(Heat)+DHW";
    case 26: return "Auto(Cool)";
    case 42: return "Auto(Cool)+DHW";
    default: 
      ESP_LOGW(TAG, "Unknown operation mode: raw=0x%02X, masked=%d", input, mode);
      return "Heat+DHW"; // Safe default instead of Unknown
  }
}

} // namespace heishamon
} // namespace esphome