#include "heishamon.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon";

void HeishamonComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Heishamon...");
  
  // Initialiser les buffers
  this->data_buffer_.reserve(MAXDATASIZE);
  this->act_data_.resize(DATASIZE, 0);
  this->act_data_extra_.resize(DATASIZE, 0);
  this->act_opt_data_.resize(OPTDATASIZE, 0);
  
  // Initialiser les requêtes prédéfinies (portées depuis HeishaMon)
  this->initial_query_ = {0x31, 0x05, 0x10, 0x01, 0x00, 0x00, 0x00};
  
  // Requête Panasonic (version simplifiée pour économiser la mémoire)
  this->panasonic_query_.resize(PANASONICQUERYSIZE, 0x00);
  this->panasonic_query_[0] = 0x71;
  this->panasonic_query_[1] = 0x6c;
  this->panasonic_query_[2] = 0x01;
  this->panasonic_query_[3] = 0x10;
  
  // Requête PCB optionnelle
  this->optional_pcb_query_ = {
    0xF1, 0x11, 0x01, 0x50, 0x00, 0x00, 0x40, 0xFF, 0xFF, 0xE5, 
    0xFF, 0xFF, 0x00, 0xFF, 0xEB, 0xFF, 0xFF, 0x00, 0x00
  };
  
  // Initialiser les topics
  this->init_topics();
  
  // Initialiser le buffer de commandes
  this->command_buffer_.resize(MAXCOMMANDSINBUFFER);
  
  ESP_LOGCONFIG(TAG, "Heishamon setup completed");
}

void HeishamonComponent::loop() {
  uint32_t now = millis();
  
  // Vérifier les timeouts
  if (this->sending_ && (now - this->send_command_read_time_) > SERIALTIMEOUT) {
    ESP_LOGW(TAG, "Command timeout, resetting send state");
    this->sending_ = false;
    this->timeout_reads_++;
  }
  
  // Lire les données série
  if (this->available() > 0) {
    this->read_serial();
  }
  
  // Traiter les commandes en buffer
  if (!this->sending_ && this->cmd_count_ > 0) {
    this->pop_command_buffer();
  }
  
  // Envoyer les requêtes périodiques
  if ((now - this->last_run_time_) > this->update_interval_) {
    this->last_run_time_ = now;
    
    if (!this->listen_only_) {
      this->send_panasonic_query();
      
      // Envoyer requête PCB optionnelle si activée
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
  LOG_UPDATE_INTERVAL(this);
}

bool HeishamonComponent::read_serial() {
  while (this->available() && this->data_buffer_.size() < MAXDATASIZE) {
    uint8_t byte;
    this->read_byte(&byte);
    
    // Premier byte = début de nouveau paquet
    if (this->data_buffer_.empty()) {
      this->total_reads_++;
      
      // Vérifier header valide
      if (byte != 0x71 && byte != 0x31 && byte != 0xF1) {
        ESP_LOGW(TAG, "Invalid header: 0x%02X", byte);
        this->bad_header_reads_++;
        return false;
      }
    }
    
    this->data_buffer_.push_back(byte);
    
    // Vérifier si on a reçu assez de données pour déterminer la longueur
    if (this->data_buffer_.size() >= 2) {
      uint8_t expected_length = this->data_buffer_[1] + 3; // longueur + header + checksum
      
      if (this->data_buffer_.size() == expected_length) {
        // Paquet complet reçu
        this->sending_ = false;
        
        if (!this->is_valid_checksum(this->data_buffer_)) {
          ESP_LOGW(TAG, "Invalid checksum");
          this->bad_crc_reads_++;
          this->data_buffer_.clear();
          return false;
        }
        
        ESP_LOGD(TAG, "Received valid packet, size: %d", this->data_buffer_.size());
        this->good_reads_++;
        
        // Décoder selon le type de données
        if (this->data_buffer_.size() == DATASIZE) {
          if (this->data_buffer_[3] == 0x10) {
            this->decode_heatpump_data(this->data_buffer_);
          } else if (this->data_buffer_[3] == 0x21) {
            this->extra_data_available_ = true;
            // Copier vers buffer extra data
            std::copy(this->data_buffer_.begin(), this->data_buffer_.end(), this->act_data_extra_.begin());
          }
        } else if (this->data_buffer_.size() == OPTDATASIZE) {
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
  
  // Calculer et ajouter checksum
  std::vector<uint8_t> query_with_checksum = this->panasonic_query_;
  uint8_t checksum = this->calc_checksum(this->panasonic_query_);
  query_with_checksum.push_back(checksum);
  
  // Envoyer
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
  // Copier vers buffer actuel
  std::copy(data.begin(), data.end(), this->act_data_.begin());
  
  // Décoder tous les topics configurés
  for (const auto &topic : this->topics_) {
    if (topic.byte_index < data.size()) {
      float value = topic.decode_func(data[topic.byte_index]);
      
      // Appeler callback si enregistré
      auto it = this->sensor_callbacks_.find(topic.name);
      if (it != this->sensor_callbacks_.end()) {
        it->second(value);
      }
    }
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
  cmd.length = std::min(command.size(), sizeof(cmd.data));
  std::copy(command.begin(), command.begin() + cmd.length, cmd.data);
  
  this->cmd_end_ = (this->cmd_end_ + 1) % MAXCOMMANDSINBUFFER;
  this->cmd_count_++;
}

void HeishamonComponent::pop_command_buffer() {
  if (this->cmd_count_ == 0) return;
  
  const CommandBuffer &cmd = this->command_buffer_[this->cmd_start_];
  std::vector<uint8_t> command(cmd.data, cmd.data + cmd.length);
  
  // Envoyer commande
  uint8_t checksum = this->calc_checksum(command);
  command.push_back(checksum);
  
  this->write_array(command);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
  
  this->cmd_start_ = (this->cmd_start_ + 1) % MAXCOMMANDSINBUFFER;
  this->cmd_count_--;
}

void HeishamonComponent::register_sensor(const std::string &topic, std::function<void(float)> callback) {
  this->sensor_callbacks_[topic] = callback;
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

// Fonctions de décodage portées depuis HeishaMon
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

void HeishamonComponent::init_topics() {
  // Initialiser les topics les plus importants (sélection des plus critiques pour économiser la mémoire)
  this->topics_.clear();
  
  // Topics principaux de la pompe à chaleur
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
  
  this->topics_.push_back({"dhw_target_temp", 42, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "DHW target temperature"});
  
  this->topics_.push_back({"outside_temp", 142, 
    [this](uint8_t input) { return this->get_int_minus_128(input); }, "°C", "Outside temperature"});
  
  this->topics_.push_back({"compressor_freq", 166, 
    [this](uint8_t input) { return this->get_int_minus_1(input); }, "Hz", "Compressor frequency"});
  
  this->topics_.push_back({"heat_power_production", 194, 
    [this](uint8_t input) { return this->get_power(input); }, "W", "Heat power production"});
  
  this->topics_.push_back({"heat_power_consumption", 193, 
    [this](uint8_t input) { return this->get_power(input); }, "W", "Heat power consumption"});
  
  this->topics_.push_back({"pump_flow", 0, 
    [this](uint8_t input) { return this->get_pump_flow(this->act_data_); }, "l/min", "Pump flow"});
  
  this->topics_.push_back({"operation_mode", 6, 
    [this](uint8_t input) { 
      // Décodage simplifié du mode opératoire
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
  
  // Topics optionnels pour PCB
  if (this->optional_pcb_) {
    this->optional_topics_.push_back({"z1_water_pump", 4, 
      [this](uint8_t input) { return static_cast<float>(input >> 7); }, "", "Zone 1 water pump"});
    
    this->optional_topics_.push_back({"z1_mixing_valve", 4, 
      [this](uint8_t input) { return static_cast<float>((input >> 5) & 0b11); }, "", "Zone 1 mixing valve"});
  }
}

}  // namespace heishamon
}  // namespace esphome
