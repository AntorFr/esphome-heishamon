#include "heishamon.h"
#include "climate.h"
#include "number.h"
#include "water_heater.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon";

void HeishamonComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heishamon...");
  
  // Initialize buffers
  this->data_buffer_.reserve(MAXDATASIZE);
  this->act_data_.resize(DATASIZE, 0);
  this->act_data_extra_.resize(DATASIZE, 0);
  this->act_opt_data_.resize(OPTDATASIZE, 0);
  
  // Initialize predefined queries (ported from HeishaMon)
  this->initial_query_ = {0x31, 0x05, 0x10, 0x01, 0x00, 0x00, 0x00};
  
  // Panasonic query (simplified version to save memory)
  this->panasonic_query_.resize(PANASONICQUERYSIZE, 0x00);
  this->panasonic_query_[0] = 0x71;
  this->panasonic_query_[1] = 0x6c;
  this->panasonic_query_[2] = 0x01;
  this->panasonic_query_[3] = 0x10;
  
  // Optional PCB query
  this->optional_pcb_query_ = {
    0xF1, 0x11, 0x01, 0x50, 0x00, 0x00, 0x40, 0xFF, 0xFF, 0xE5, 
    0xFF, 0xFF, 0x00, 0xFF, 0xEB, 0xFF, 0xFF, 0x00, 0x00
  };
  
  // Initialize topics
  this->init_topics();
  
  // Initialize command buffer
  this->command_buffer_.resize(MAXCOMMANDSINBUFFER);
  
  ESP_LOGCONFIG(TAG, "Heishamon setup completed");
}

void HeishamonComponent::loop() {
  uint32_t now = millis();
  
  // Check timeouts
  if (this->sending_ && (now - this->send_command_read_time_) > SERIALTIMEOUT) {
    ESP_LOGW(TAG, "Command timeout, resetting send state");
    this->sending_ = false;
    this->timeout_reads_++;
  }
  
  // Read serial data
  if (this->available() > 0) {
    this->read_serial();
  }
  
  // Process buffered commands
  if (!this->sending_ && this->cmd_count_ > 0) {
    this->pop_command_buffer();
  }
  
  // Send periodic queries
  if ((now - this->last_run_time_) > this->update_interval_) {
    this->last_run_time_ = now;
    
    if (!this->listen_only_) {
      this->send_panasonic_query();
      
      // Send optional PCB query if enabled
      if (this->optional_pcb_ && (now - this->last_optional_pcb_time_) > 1000) {
        this->last_optional_pcb_time_ = now;
        this->send_optional_pcb_query();
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

bool HeishamonComponent::read_serial() {
  while (this->available() && this->data_buffer_.size() < MAXDATASIZE) {
    uint8_t byte;
    this->read_byte(&byte);
    
    // First byte = start of new packet
    if (this->data_buffer_.empty()) {
      this->total_reads_++;
      
      // Check valid header
      if (byte != 0x71 && byte != 0x31 && byte != 0xF1) {
        ESP_LOGW(TAG, "Invalid header: 0x%02X", byte);
        this->bad_header_reads_++;
        return false;
      }
      ESP_LOGV(TAG, "Found valid header: 0x%02X", byte);
    }
    
    this->data_buffer_.push_back(byte);
    
    // Check if we received enough data to determine length
    if (this->data_buffer_.size() >= 2) {
      uint8_t expected_length = this->data_buffer_[1] + 3; // length + header + checksum
      
      if (this->data_buffer_.size() == expected_length) {
        // Complete packet received
        this->sending_ = false;
        
        if (!this->is_valid_checksum(this->data_buffer_)) {
          ESP_LOGW(TAG, "Invalid checksum");
          this->bad_crc_reads_++;
          this->data_buffer_.clear();
          return false;
        }
        
        ESP_LOGD(TAG, "Received valid packet, size: %d, header: 0x%02X", 
                 this->data_buffer_.size(), this->data_buffer_[0]);
        this->good_reads_++;
        
        // Decode according to data type
        if (this->data_buffer_.size() == DATASIZE) {
          if (this->data_buffer_[3] == 0x10) {
            ESP_LOGD(TAG, "Decoding heatpump data packet (0x10)");
            this->decode_heatpump_data(this->data_buffer_);
          } else if (this->data_buffer_[3] == 0x21) {
            this->extra_data_available_ = true;
            ESP_LOGD(TAG, "Received extra data packet (0x21)");
            // Copy to extra data buffer
            std::copy(this->data_buffer_.begin(), this->data_buffer_.end(), this->act_data_extra_.begin());
          }
        } else if (this->data_buffer_.size() == OPTDATASIZE) {
          ESP_LOGD(TAG, "Decoding optional data packet");
          this->decode_optional_data(this->data_buffer_);
        }
        
        this->data_buffer_.clear();
        return true;
      } else if (this->data_buffer_.size() > expected_length) {
        ESP_LOGW(TAG, "Received more data than expected");
        this->data_buffer_.clear();
        return false;
      }
    }
  }
  
  return false;
}

void HeishamonComponent::send_panasonic_query() {
  if (this->sending_) {
    ESP_LOGW(TAG, "Already sending, buffering query");
    this->push_command_buffer(this->panasonic_query_);
    return;
  }
  
  ESP_LOGD(TAG, "Sending panasonic query");
  
  // Calculate and add checksum
  std::vector<uint8_t> query_with_checksum = this->panasonic_query_;
  uint8_t checksum = this->calc_checksum(this->panasonic_query_);
  query_with_checksum.push_back(checksum);
  
  // Send
  this->write_array(query_with_checksum);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
}

void HeishamonComponent::send_optional_pcb_query() {
  if (this->sending_) {
    this->push_command_buffer(this->optional_pcb_query_);
    return;
  }
  
  ESP_LOGD(TAG, "Sending optional PCB query");
  
  std::vector<uint8_t> query_with_checksum = this->optional_pcb_query_;
  uint8_t checksum = this->calc_checksum(this->optional_pcb_query_);
  query_with_checksum.push_back(checksum);
  
  this->write_array(query_with_checksum);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
}

void HeishamonComponent::decode_heatpump_data(const std::vector<uint8_t> &data) {
  // Copy to current buffer
  std::copy(data.begin(), data.end(), this->act_data_.begin());
  
  ESP_LOGD(TAG, "Decoding heatpump data, %d topics configured", this->topics_.size());
  
  // Decode all configured topics
  for (const auto &topic : this->topics_) {
    if (topic.byte_index < data.size()) {
      float value = topic.decode_func(data[topic.byte_index]);
      
      // Special debug for DHW temperature
      if (topic.name == "dhw_temp") {
        ESP_LOGD(TAG, "DHW Temperature: byte_index=%d, raw_byte=0x%02X, decoded_value=%.1f", 
                 topic.byte_index, data[topic.byte_index], value);
      }
      
      // Update internal temperature values for climate components
      if (topic.name == "z1_temp") {
        this->zone1_current_temp_ = value;
      } else if (topic.name == "z2_temp") {
        this->zone2_current_temp_ = value;
      } else if (topic.name == "z1_heat_request_temp") {
        this->zone1_heat_target_temp_ = value;
      } else if (topic.name == "z1_cool_request_temp") {
        this->zone1_cool_target_temp_ = value;
      } else if (topic.name == "z2_heat_request_temp") {
        this->zone2_heat_target_temp_ = value;
      } else if (topic.name == "z2_cool_request_temp") {
        this->zone2_cool_target_temp_ = value;
      } else if (topic.name == "dhw_temp") {
        this->dhw_current_temp_ = value;
        ESP_LOGD(TAG, "DHW Temperature updated internally: %.1f°C", value);
      } else if (topic.name == "dhw_target_temp") {
        this->dhw_target_temp_ = value;
      } else if (topic.name == "dhw_heating") {
        this->dhw_heating_state_ = (static_cast<int>(value) == 1);
      } else if (topic.name == "dhw_mode") {
        this->dhw_mode_ = static_cast<int>(value);
      } else if (topic.name == "operating_mode_state") {
        // Update operating mode states based on value
        this->heat_mode_enabled_ = (static_cast<int>(value) & 0x01) != 0;
        this->cool_mode_enabled_ = (static_cast<int>(value) & 0x02) != 0;
      } else if (topic.name == "zones_state") {
        // Update zone states based on value
        int zone_state = static_cast<int>(value);
        this->zone1_heat_enabled_ = (zone_state & 0x01) != 0;
        this->zone2_heat_enabled_ = (zone_state & 0x02) != 0;
        // For simplicity, assume same for cooling - could be refined
        this->zone1_cool_enabled_ = (zone_state & 0x01) != 0;
        this->zone2_cool_enabled_ = (zone_state & 0x02) != 0;
      }
      
      // Call callback if registered
      auto it = this->sensor_callbacks_.find(topic.name);
      if (it != this->sensor_callbacks_.end()) {
        ESP_LOGD(TAG, "Calling sensor callback for %s with value %.1f", topic.name.c_str(), value);
        it->second(value);
      } else {
        // Debug: mention if no callback is registered
        if (topic.name == "dhw_temp") {
          ESP_LOGW(TAG, "No callback registered for dhw_temp sensor!");
        }
      }
    } else {
      ESP_LOGW(TAG, "Topic %s: byte_index %d is out of bounds (data size: %d)", 
               topic.name.c_str(), topic.byte_index, data.size());
    }
  }
  
  // Update all registered climate components
  for (auto *climate : this->climate_components_) {
    climate->update_from_heishamon();
  }
  
  // Update all registered water heater components
  for (auto *water_heater : this->water_heater_components_) {
    water_heater->update_current_temperature(this->dhw_current_temp_);
    water_heater->update_target_temperature(this->dhw_target_temp_);
    water_heater->update_dhw_state(this->dhw_heating_state_);
    water_heater->update_dhw_mode(this->dhw_mode_);
  }
}

void HeishamonComponent::decode_optional_data(const std::vector<uint8_t> &data) {
  std::copy(data.begin(), data.begin() + OPTDATASIZE, this->act_opt_data_.begin());
  
  for (const auto &topic : this->optional_topics_) {
    if (topic.byte_index < data.size()) {
      float value = topic.decode_func(data[topic.byte_index]);
      
      auto it = this->sensor_callbacks_.find(topic.name);
      if (it != this->sensor_callbacks_.end()) {
        it->second(value);
      }
    }
  }
}

uint8_t HeishamonComponent::calc_checksum(const std::vector<uint8_t> &command) {
  uint8_t chk = 0;
  for (uint8_t byte : command) {
    chk += byte;
  }
  chk = (chk ^ 0xFF) + 1;
  return chk;
}

bool HeishamonComponent::is_valid_checksum(const std::vector<uint8_t> &data) {
  uint8_t chk = 0;
  for (uint8_t byte : data) {
    chk += byte;
  }
  return (chk == 0);
}

void HeishamonComponent::push_command_buffer(const std::vector<uint8_t> &command) {
  if (this->cmd_count_ >= MAXCOMMANDSINBUFFER) {
    ESP_LOGW(TAG, "Command buffer full, dropping command");
    return;
  }
  
  CommandBuffer &cmd = this->command_buffer_[this->cmd_end_];
  cmd.size = std::min(command.size(), sizeof(cmd.data));
  std::copy(command.begin(), command.begin() + cmd.size, cmd.data);
  
  this->cmd_end_ = (this->cmd_end_ + 1) % MAXCOMMANDSINBUFFER;
  this->cmd_count_++;
}

void HeishamonComponent::pop_command_buffer() {
  if (this->cmd_count_ == 0) return;
  
  const CommandBuffer &cmd = this->command_buffer_[this->cmd_start_];
  std::vector<uint8_t> command(cmd.data, cmd.data + cmd.size);
  
  // Send command
  uint8_t checksum = this->calc_checksum(command);
  command.push_back(checksum);
  
  this->write_array(command);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
  
  this->cmd_start_ = (this->cmd_start_ + 1) % MAXCOMMANDSINBUFFER;
  this->cmd_count_--;
}

void HeishamonComponent::register_sensor_callback(const std::string &topic, std::function<void(float)> &&callback) {
  ESP_LOGD(TAG, "Registering sensor callback for topic: %s", topic.c_str());
  this->sensor_callbacks_[topic] = std::move(callback);
  ESP_LOGD(TAG, "Total sensor callbacks registered: %d", this->sensor_callbacks_.size());
}

bool HeishamonComponent::send_command(const std::vector<uint8_t> &command) {
  if (this->listen_only_) {
    ESP_LOGW(TAG, "Cannot send command in listen-only mode");
    return false;
  }
  
  if (this->sending_) {
    this->push_command_buffer(command);
    return true;
  }
  
  std::vector<uint8_t> cmd_with_checksum = command;
  uint8_t checksum = this->calc_checksum(command);
  cmd_with_checksum.push_back(checksum);
  
  this->write_array(cmd_with_checksum);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
  
  return true;
}

void HeishamonComponent::send_command(const std::string &command, const std::string &value) {
  ESP_LOGD(TAG, "Sending string command: %s = %s", command.c_str(), value.c_str());
  
  if (this->listen_only_) {
    ESP_LOGW(TAG, "Cannot send command in listen-only mode");
    return;
  }
  
  // Convert string command to appropriate byte command
  // This is a simplified implementation - in practice, you'd have a mapping
  // of command names to their corresponding byte sequences
  
  if (command == "SetBivalentMode") {
    uint8_t val = static_cast<uint8_t>(std::stoi(value));
    this->create_command("SetBivalentMode", val);
  } else if (command == "SetExternalPadHeater") {
    uint8_t val = static_cast<uint8_t>(std::stoi(value));
    this->create_command("SetExternalPadHeater", val);
  } else if (command == "SetSmartGridMode") {
    uint8_t val = static_cast<uint8_t>(std::stoi(value));
    this->create_command("SetSmartGridMode", val);
  } else if (command == "SetHeatingMode") {
    uint8_t val = static_cast<uint8_t>(std::stoi(value));
    this->create_command("SetHeatingMode", val);
  } else if (command == "SetCoolingMode") {
    uint8_t val = static_cast<uint8_t>(std::stoi(value));
    this->create_command("SetCoolingMode", val);
  } else {
    ESP_LOGW(TAG, "Unknown string command: %s", command.c_str());
  }
}

bool HeishamonComponent::send_number_command(const std::string &command, float value) {
  ESP_LOGD(TAG, "Sending number command: %s = %.1f", command.c_str(), value);
  
  if (this->listen_only_) {
    ESP_LOGW(TAG, "Cannot send command in listen-only mode");
    return false;
  }
  
  // Convert float to integer for HeishaMon protocol
  uint8_t int_value = static_cast<uint8_t>(value);
  
  // Map number commands to protocol commands
  if (command == "SetDHWTargetTemp") {
    this->create_command("SetDHWTargetTemp", int_value);
    return true;
  } else if (command == "SetZ1HeatTargetTemp") {
    this->create_command("SetZ1HeatTargetTemp", int_value);
    return true;
  } else if (command == "SetZ2HeatTargetTemp") {
    this->create_command("SetZ2HeatTargetTemp", int_value);
    return true;
  } else if (command == "SetZ1CoolTargetTemp") {
    this->create_command("SetZ1CoolTargetTemp", int_value);
    return true;
  } else if (command == "SetZ2CoolTargetTemp") {
    this->create_command("SetZ2CoolTargetTemp", int_value);
    return true;
  } else {
    ESP_LOGW(TAG, "Unknown number command: %s", command.c_str());
    return false;
  }
}

// Decoding functions ported from HeishaMon
float HeishamonComponent::unknown(uint8_t input) { return -1; }

float HeishamonComponent::get_bit_1_and_2(uint8_t input) {
  return (input >> 6) - 1;
}

float HeishamonComponent::get_bit_3_and_4(uint8_t input) {
  return ((input >> 4) & 0b11) - 1;
}

float HeishamonComponent::get_bit_5_and_6(uint8_t input) {
  return ((input >> 2) & 0b11) - 1;
}

float HeishamonComponent::get_bit_7_and_8(uint8_t input) {
  return (input & 0b11) - 1;
}

float HeishamonComponent::get_int_minus_1(uint8_t input) {
  return static_cast<float>(input - 1);
}

float HeishamonComponent::get_int_minus_128(uint8_t input) {
  return static_cast<float>(input - 128);
}

float HeishamonComponent::get_int_minus_1_div_5(uint8_t input) {
  return (static_cast<float>(input - 1)) / 5.0f;
}

float HeishamonComponent::get_int_minus_1_div_50(uint8_t input) {
  return (static_cast<float>(input - 1)) / 50.0f;
}

float HeishamonComponent::get_int_minus_1_times_10(uint8_t input) {
  return static_cast<float>((input - 1) * 10);
}

float HeishamonComponent::get_int_minus_1_times_50(uint8_t input) {
  return static_cast<float>((input - 1) * 50);
}

float HeishamonComponent::get_power(uint8_t input) {
  return static_cast<float>((input - 1) * 200);
}

float HeishamonComponent::get_pump_flow(const std::vector<uint8_t> &data) {
  if (data.size() > 170) {
    float pump_flow1 = static_cast<float>(data[170]);
    float pump_flow2 = (static_cast<float>(data[169] - 1)) / 256.0f;
    return pump_flow1 + pump_flow2;
  }
  return -1;
}

// Additional decoding functions for Phase 1 sensors
float HeishamonComponent::get_dhw_power(uint8_t input) {
  // DHW power calculation (similar to general power but might have different scale)
  return static_cast<float>((input - 1) * 200);
}

float HeishamonComponent::get_heat_delta(uint8_t input) {
  // Heat delta temperature calculation (in 0.5°C steps)
  return (static_cast<float>(input - 1)) / 2.0f;
}

float HeishamonComponent::get_cool_delta(uint8_t input) {
  // Cool delta temperature calculation (in 0.5°C steps)
  return (static_cast<float>(input - 1)) / 2.0f;
}

float HeishamonComponent::get_operating_hours(uint8_t input) {
  // Operating hours (might need to be converted to proper hours)
  return static_cast<float>(input);
}

float HeishamonComponent::get_zone_valve_pid(uint8_t input) {
  // Zone valve PID percentage (0-100%)
  return static_cast<float>(input);
}

float HeishamonComponent::get_cop(const std::vector<uint8_t> &data) {
  // COP calculation: Heat production / Heat consumption
  if (data.size() > 194) {
    float heat_production = this->get_power(data[194]);
    float heat_consumption = this->get_power(data[193]);
    
    // Avoid division by zero and ensure meaningful COP
    if (heat_consumption > 100.0f && heat_production > 0.0f) {
      float cop = heat_production / heat_consumption;
      // Reasonable COP bounds (1.0 to 8.0)
      if (cop >= 1.0f && cop <= 8.0f) {
        return cop;
      }
    }
  }
  return -1.0f; // Invalid COP
}

void HeishamonComponent::init_topics() {
  // Initialize the most important topics (selection of critical ones to save memory)
  this->topics_.clear();
  
  ESP_LOGD(TAG, "Initializing topics...");
  
  // Basic heat pump topics
  this->topics_.push_back({"heatpump_state", 4, 
    [this](uint8_t input) { return this->get_bit_7_and_8(input); }, "", "Heat pump state"});
  
  this->topics_.push_back({"main_inlet_temp", 143, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Main inlet temperature"});
  
  this->topics_.push_back({"main_outlet_temp", 144, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Main outlet temperature"});
  
  this->topics_.push_back({"main_target_temp", 153, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Main target temperature"});
  
  this->topics_.push_back({"dhw_temp", 141, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "DHW temperature"});
  ESP_LOGD(TAG, "DHW Temperature topic initialized at byte_index 141");
  
  this->topics_.push_back({"dhw_target_temp", 42, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "DHW target temperature"});
  
  this->topics_.push_back({"outside_temp", 142, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Outside temperature"});
  
  this->topics_.push_back({"compressor_freq", 166, 
    [this](uint8_t input) { return this->get_int_minus_1(input); }, "Hz", "Compressor frequency"});
  
  this->topics_.push_back({"operation_mode", 6, 
    [this](uint8_t input) { 
      // Simplified operation mode decoding
      uint8_t mode = input & 0b111111;
      switch (mode) {
        case 18: return 0.0f; // Heat
        case 19: return 1.0f; // Cool
        case 25: return 2.0f; // Auto(heat)
        case 33: return 3.0f; // DHW
        case 34: return 4.0f; // Heat+DHW
        case 35: return 5.0f; // Cool+DHW
        case 41: return 6.0f; // Auto(heat)+DHW
        case 26: return 7.0f; // Auto(cool)
        case 42: return 8.0f; // Auto(cool)+DHW
        default: return -1.0f;
      }
    }, "", "Operation mode"});
  
  this->topics_.push_back({"pump_flow", 0, 
    [this](uint8_t input) { return this->get_pump_flow(this->act_data_); }, "l/min", "Pump flow"});

  // PHASE 1: Advanced power sensors (separated by function)
  this->topics_.push_back({"heat_power_production", 194, 
    [this](uint8_t input) { return this->get_power(input); }, "W", "Heat power production"});
  
  this->topics_.push_back({"heat_power_consumption", 193, 
    [this](uint8_t input) { return this->get_power(input); }, "W", "Heat power consumption"});
  
  // DHW power sensors (if available in your heat pump model)
  this->topics_.push_back({"dhw_power_production", 195, 
    [this](uint8_t input) { return this->get_dhw_power(input); }, "W", "DHW power production"});
  
  this->topics_.push_back({"dhw_power_consumption", 196, 
    [this](uint8_t input) { return this->get_dhw_power(input); }, "W", "DHW power consumption"});
  
  // Cool power sensors (for cooling capable units)
  this->topics_.push_back({"cool_power_production", 197, 
    [this](uint8_t input) { return this->get_power(input); }, "W", "Cool power production"});
  
  this->topics_.push_back({"cool_power_consumption", 198, 
    [this](uint8_t input) { return this->get_power(input); }, "W", "Cool power consumption"});

  // Temperature deltas (TOP23, TOP24 in HA module)
  this->topics_.push_back({"heat_delta", 23, 
    [this](uint8_t input) { return this->get_heat_delta(input); }, "°C", "Heat delta temperature"});
  
  this->topics_.push_back({"cool_delta", 24, 
    [this](uint8_t input) { return this->get_cool_delta(input); }, "°C", "Cool delta temperature"});

  // Zone temperatures (TOP36, TOP37 in HA module)
  this->topics_.push_back({"z1_water_temp", 36, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Zone 1 water temperature"});
  
  this->topics_.push_back({"z2_water_temp", 37, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Zone 2 water temperature"});
  
  this->topics_.push_back({"room_thermostat_temp", 33, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Room thermostat temperature"});

  // Operating hours (TOP90, TOP91, TOP88 in HA module)
  this->topics_.push_back({"room_heater_operating_hours", 90, 
    [this](uint8_t input) { return this->get_operating_hours(input); }, "h", "Room heater operating hours"});
  
  this->topics_.push_back({"dhw_heater_operating_hours", 91, 
    [this](uint8_t input) { return this->get_operating_hours(input); }, "h", "DHW heater operating hours"});
  
  this->topics_.push_back({"compressor_operating_hours", 88, 
    [this](uint8_t input) { return this->get_operating_hours(input); }, "h", "Compressor operating hours"});

  // Holiday shift temperatures (TOP45, TOP25 in HA module)
  this->topics_.push_back({"room_holiday_shift_temp", 45, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Room holiday shift temperature"});
  
  this->topics_.push_back({"dhw_holiday_shift_temp", 25, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "DHW holiday shift temperature"});

  // Buffer temperature (TOP46 in HA module)
  this->topics_.push_back({"buffer_temp", 46, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Buffer temperature"});

  // Zone valve PID control (TOP127, TOP128 in HA module)
  this->topics_.push_back({"z1_valve_pid", 127, 
    [this](uint8_t input) { return this->get_zone_valve_pid(input); }, "%", "Zone 1 valve PID"});
  
  this->topics_.push_back({"z2_valve_pid", 128, 
    [this](uint8_t input) { return this->get_zone_valve_pid(input); }, "%", "Zone 2 valve PID"});

  // COP calculation (computed from multiple values)
  this->topics_.push_back({"cop", 0, 
    [this](uint8_t input) { return this->get_cop(this->act_data_); }, "", "Coefficient of Performance"});
  
  // Optional topics for PCB
  if (this->optional_pcb_) {
    this->optional_topics_.push_back({"z1_water_pump", 4, 
      [this](uint8_t input) { return static_cast<float>(input >> 7); }, "", "Zone 1 water pump"});
    
    this->optional_topics_.push_back({"z1_mixing_valve", 4, 
      [this](uint8_t input) { return static_cast<float>((input >> 5) & 0b11); }, "", "Zone 1 mixing valve"});
  }
}

// Climate control implementation
void HeishamonComponent::register_climate_component(HeishaMonClimate *climate) {
  this->climate_components_.push_back(climate);
}

// Heat pump mode control
void HeishamonComponent::set_heat_mode_enabled(bool enabled) {
  if (this->heat_mode_enabled_ != enabled) {
    this->heat_mode_enabled_ = enabled;
    // Send command to enable/disable heat mode globally
    this->create_command("SetOperationMode", enabled ? 1 : 0);
  }
}

void HeishamonComponent::set_cool_mode_enabled(bool enabled) {
  if (this->cool_mode_enabled_ != enabled) {
    this->cool_mode_enabled_ = enabled;
    // Send command to enable/disable cool mode globally  
    this->create_command("SetOperationMode", enabled ? 2 : 0);
  }
}

// Zone control
void HeishamonComponent::set_zone1_heat_enabled(bool enabled) {
  if (this->zone1_heat_enabled_ != enabled) {
    this->zone1_heat_enabled_ = enabled;
    // Update zone states - Zone 1 bit
    uint8_t zone_state = 0;
    if (this->zone1_heat_enabled_ || this->zone1_cool_enabled_) zone_state |= 0x01;
    if (this->zone2_heat_enabled_ || this->zone2_cool_enabled_) zone_state |= 0x02;
    this->create_command("SetZones", zone_state);
  }
}

void HeishamonComponent::set_zone1_cool_enabled(bool enabled) {
  if (this->zone1_cool_enabled_ != enabled) {
    this->zone1_cool_enabled_ = enabled;
    // Update zone states - Zone 1 bit
    uint8_t zone_state = 0;
    if (this->zone1_heat_enabled_ || this->zone1_cool_enabled_) zone_state |= 0x01;
    if (this->zone2_heat_enabled_ || this->zone2_cool_enabled_) zone_state |= 0x02;
    this->create_command("SetZones", zone_state);
  }
}

void HeishamonComponent::set_zone2_heat_enabled(bool enabled) {
  if (this->zone2_heat_enabled_ != enabled) {
    this->zone2_heat_enabled_ = enabled;
    // Update zone states - Zone 2 bit
    uint8_t zone_state = 0;
    if (this->zone1_heat_enabled_ || this->zone1_cool_enabled_) zone_state |= 0x01;
    if (this->zone2_heat_enabled_ || this->zone2_cool_enabled_) zone_state |= 0x02;
    this->create_command("SetZones", zone_state);
  }
}

void HeishamonComponent::set_zone2_cool_enabled(bool enabled) {
  if (this->zone2_cool_enabled_ != enabled) {
    this->zone2_cool_enabled_ = enabled;
    // Update zone states - Zone 2 bit
    uint8_t zone_state = 0;
    if (this->zone1_heat_enabled_ || this->zone1_cool_enabled_) zone_state |= 0x01;
    if (this->zone2_heat_enabled_ || this->zone2_cool_enabled_) zone_state |= 0x02;
    this->create_command("SetZones", zone_state);
  }
}

// Temperature setters
void HeishamonComponent::set_zone1_heat_target_temperature(float temperature) {
  if (fabsf(this->zone1_heat_target_temp_ - temperature) > 0.1f) {
    this->zone1_heat_target_temp_ = temperature;
    // Convert to heat pump protocol (typically temperature * 2 + offset)
    uint8_t temp_value = static_cast<uint8_t>(temperature);
    this->create_command("SetZ1HeatRequestTemperature", temp_value);
  }
}

void HeishamonComponent::set_zone1_cool_target_temperature(float temperature) {
  if (fabsf(this->zone1_cool_target_temp_ - temperature) > 0.1f) {
    this->zone1_cool_target_temp_ = temperature;
    uint8_t temp_value = static_cast<uint8_t>(temperature);
    this->create_command("SetZ1CoolRequestTemperature", temp_value);
  }
}

void HeishamonComponent::set_zone2_heat_target_temperature(float temperature) {
  if (fabsf(this->zone2_heat_target_temp_ - temperature) > 0.1f) {
    this->zone2_heat_target_temp_ = temperature;
    uint8_t temp_value = static_cast<uint8_t>(temperature);
    this->create_command("SetZ2HeatRequestTemperature", temp_value);
  }
}

void HeishamonComponent::set_zone2_cool_target_temperature(float temperature) {
  if (fabsf(this->zone2_cool_target_temp_ - temperature) > 0.1f) {
    this->zone2_cool_target_temp_ = temperature;
    uint8_t temp_value = static_cast<uint8_t>(temperature);
    this->create_command("SetZ2CoolRequestTemperature", temp_value);
  }
}

// Temperature getters
float HeishamonComponent::get_zone1_current_temperature() const {
  return this->zone1_current_temp_;
}

float HeishamonComponent::get_zone2_current_temperature() const {
  return this->zone2_current_temp_;
}

float HeishamonComponent::get_zone1_heat_target_temperature() const {
  return this->zone1_heat_target_temp_;
}

float HeishamonComponent::get_zone1_cool_target_temperature() const {
  return this->zone1_cool_target_temp_;
}

float HeishamonComponent::get_zone2_heat_target_temperature() const {
  return this->zone2_heat_target_temp_;
}

float HeishamonComponent::get_zone2_cool_target_temperature() const {
  return this->zone2_cool_target_temp_;
}

// State getters
bool HeishamonComponent::get_heat_mode_enabled() const {
  return this->heat_mode_enabled_;
}

bool HeishamonComponent::get_cool_mode_enabled() const {
  return this->cool_mode_enabled_;
}

bool HeishamonComponent::get_zone1_heat_enabled() const {
  return this->zone1_heat_enabled_;
}

bool HeishamonComponent::get_zone1_cool_enabled() const {
  return this->zone1_cool_enabled_;
}

bool HeishamonComponent::get_zone2_heat_enabled() const {
  return this->zone2_heat_enabled_;
}

bool HeishamonComponent::get_zone2_cool_enabled() const {
  return this->zone2_cool_enabled_;
}

// Number component support
void HeishamonComponent::register_number(HeishamonNumber *number) {
  this->number_components_.push_back(number);
}

// Water Heater component registration and support methods
void HeishamonComponent::register_water_heater(HeishamonWaterHeater *water_heater) {
  this->water_heater_components_.push_back(water_heater);
  water_heater->set_parent(this);
  ESP_LOGD(TAG, "Registered Water Heater component");
}

float HeishamonComponent::get_dhw_current_temperature() const {
  return this->dhw_current_temp_;
}

float HeishamonComponent::get_dhw_target_temperature() const {
  return this->dhw_target_temp_;
}

bool HeishamonComponent::get_dhw_heating_state() const {
  return this->dhw_heating_state_;
}

int HeishamonComponent::get_dhw_mode() const {
  return this->dhw_mode_;
}

// Missing method implementations for linkage
void HeishamonComponent::register_binary_sensor_callback(const std::string &topic, std::function<void(bool)> callback) {
  this->binary_sensor_callbacks_[topic] = callback;
  ESP_LOGD(TAG, "Registered binary sensor callback for topic: %s", topic.c_str());
}

void HeishamonComponent::create_command(const std::string &command, uint8_t value) {
  ESP_LOGD(TAG, "Creating command: %s = %d", command.c_str(), value);
  
  if (this->listen_only_) {
    ESP_LOGW(TAG, "Cannot send command in listen-only mode");
    return;
  }
  
  // This is a simplified command mapping - in a real implementation,
  // you would have a comprehensive mapping of command names to their
  // corresponding byte sequences according to the HeishaMon protocol
  
  std::vector<uint8_t> cmd;
  
  // Basic command structure for HeishaMon protocol
  cmd.push_back(0xF1);  // Start byte for command
  cmd.push_back(0x06);  // Command length (example)
  cmd.push_back(0x01);  // Command type (example)
  cmd.push_back(0x30);  // Command group (example)
  
  // Map command name to specific command bytes
  if (command == "SetOperationMode") {
    cmd.push_back(0x01);  // Operation mode command
    cmd.push_back(value);
  } else if (command == "SetZones") {
    cmd.push_back(0x02);  // Zone control command
    cmd.push_back(value);
  } else if (command == "SetZ1HeatRequestTemperature") {
    cmd.push_back(0x03);  // Zone 1 heat temperature command
    cmd.push_back(value);
  } else if (command == "SetZ1CoolRequestTemperature") {
    cmd.push_back(0x04);  // Zone 1 cool temperature command
    cmd.push_back(value);
  } else if (command == "SetZ2HeatRequestTemperature") {
    cmd.push_back(0x05);  // Zone 2 heat temperature command
    cmd.push_back(value);
  } else if (command == "SetZ2CoolRequestTemperature") {
    cmd.push_back(0x06);  // Zone 2 cool temperature command
    cmd.push_back(value);
  } else if (command == "SetDHWTargetTemp") {
    cmd.push_back(0x07);  // DHW target temperature command
    cmd.push_back(value);
  } else if (command == "SetBivalentMode") {
    cmd.push_back(0x08);  // Bivalent mode command
    cmd.push_back(value);
  } else if (command == "SetExternalPadHeater") {
    cmd.push_back(0x09);  // External pad heater command
    cmd.push_back(value);
  } else if (command == "SetSmartGridMode") {
    cmd.push_back(0x0A);  // Smart grid mode command
    cmd.push_back(value);
  } else if (command == "SetHeatingMode") {
    cmd.push_back(0x0B);  // Heating mode command
    cmd.push_back(value);
  } else if (command == "SetCoolingMode") {
    cmd.push_back(0x0C);  // Cooling mode command
    cmd.push_back(value);
  } else {
    ESP_LOGW(TAG, "Unknown command: %s", command.c_str());
    return;
  }
  
  // Send the command
  this->send_command(cmd);
}

}  // namespace heishamon
}  // namespace esphome
