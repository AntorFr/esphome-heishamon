#include "heishamon_protocol.h"

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.protocol";

HeishamonProtocol::HeishamonProtocol(uart::UARTDevice *uart) : uart_(uart) {
  // Reserve buffers
  this->data_buffer_.reserve(MAXDATASIZE);
  this->act_data_.resize(DATASIZE, 0);
  this->act_data_extra_.resize(DATASIZE, 0);
  this->act_opt_data_.resize(OPTDATASIZE, 0);
}

void HeishamonProtocol::init() {
  ESP_LOGCONFIG(TAG, "Initializing HeishaMon Protocol...");
  
  this->init_queries();
  
  ESP_LOGCONFIG(TAG, "HeishaMon Protocol initialized");
}

void HeishamonProtocol::init_queries() {
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
}

void HeishamonProtocol::process_loop() {
  uint32_t now = millis();
  
  // Debug: Log periodically to confirm protocol is running
  static uint32_t last_debug = 0;
  if ((now - last_debug) > 10000) { // Every 10 seconds
    last_debug = now;
    ESP_LOGV(TAG, "Protocol stats - total: %u, good: %u, bad_header: %u, bad_crc: %u, timeout: %u", 
             this->total_reads_, this->good_reads_, this->bad_header_reads_, 
             this->bad_crc_reads_, this->timeout_reads_);
    ESP_LOGV(TAG, "UART available: %d, sending: %s", this->uart_->available(), YESNO(this->sending_));
  }
  
  this->check_timeouts();
  
  // Read serial data
  if (this->uart_->available() > 0) {
    ESP_LOGVV(TAG, "UART data available: %d bytes", this->uart_->available());
    this->read_serial();
  }
  
  // Process buffered commands
  this->process_commands();
}

void HeishamonProtocol::check_timeouts() {
  uint32_t now = millis();
  
  if (this->sending_ && (now - this->send_command_read_time_) > SERIALTIMEOUT) {
    ESP_LOGW(TAG, "Command timeout, resetting send state");
    this->sending_ = false;
    this->timeout_reads_++;
  }
}

void HeishamonProtocol::process_commands() {
  if (!this->sending_ && this->cmd_count_ > 0) {
    this->pop_command_buffer();
  }
}

bool HeishamonProtocol::read_serial() {
  ESP_LOGVV(TAG, "read_serial() called - available bytes: %d", this->uart_->available());
  
  while (this->uart_->available() && this->data_buffer_.size() < MAXDATASIZE) {
    uint8_t byte;
    this->uart_->read_byte(&byte);
    
    ESP_LOGV(TAG, "Read byte: 0x%02X", byte);
    
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
        
        ESP_LOGV(TAG, "Received valid packet, size: %d, header: 0x%02X", 
                 this->data_buffer_.size(), this->data_buffer_[0]);
        this->good_reads_++;
        
        // Store data and notify callback
        if (this->data_buffer_.size() == DATASIZE) {
          if (this->data_buffer_[3] == DATA_TYPE_NORMAL) {
            ESP_LOGV(TAG, "Received normal data packet (0x10)");
            std::copy(this->data_buffer_.begin(), this->data_buffer_.end(), this->act_data_.begin());
            if (this->data_callback_) {
              this->data_callback_(this->data_buffer_, DATA_TYPE_NORMAL);
            }
          } else if (this->data_buffer_[3] == DATA_TYPE_EXTRA) {
            this->extra_data_available_ = true;
            ESP_LOGV(TAG, "Received extra data packet (0x21)");
            std::copy(this->data_buffer_.begin(), this->data_buffer_.end(), this->act_data_extra_.begin());
            if (this->data_callback_) {
              this->data_callback_(this->data_buffer_, DATA_TYPE_EXTRA);
            }
          }
        } else if (this->data_buffer_.size() == OPTDATASIZE) {
          ESP_LOGD(TAG, "Received optional data packet");
          std::copy(this->data_buffer_.begin(), this->data_buffer_.begin() + OPTDATASIZE, this->act_opt_data_.begin());
          if (this->data_callback_) {
            this->data_callback_(this->data_buffer_, DATA_TYPE_OPTIONAL);
          }
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

void HeishamonProtocol::send_initial_query() {
  if (this->sending_) {
    ESP_LOGW(TAG, "Already sending, buffering initial query");
    this->push_command_buffer(this->initial_query_);
    return;
  }
  
  ESP_LOGD(TAG, "Sending initial query");
  
  // Calculate and add checksum
  std::vector<uint8_t> query_with_checksum = this->initial_query_;
  uint8_t checksum = this->calc_checksum(this->initial_query_);
  query_with_checksum.push_back(checksum);
  
  // Send
  this->uart_->write_array(query_with_checksum);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
}

void HeishamonProtocol::send_panasonic_query() {
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
  this->uart_->write_array(query_with_checksum);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
}

void HeishamonProtocol::send_optional_pcb_query() {
  if (this->sending_) {
    this->push_command_buffer(this->optional_pcb_query_);
    return;
  }
  
  ESP_LOGD(TAG, "Sending optional PCB query");
  
  std::vector<uint8_t> query_with_checksum = this->optional_pcb_query_;
  uint8_t checksum = this->calc_checksum(this->optional_pcb_query_);
  query_with_checksum.push_back(checksum);
  
  this->uart_->write_array(query_with_checksum);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
}

bool HeishamonProtocol::send_command(const std::vector<uint8_t> &command) {
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
  
  this->uart_->write_array(cmd_with_checksum);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
  
  return true;
}

uint8_t HeishamonProtocol::calc_checksum(const std::vector<uint8_t> &command) {
  uint8_t chk = 0;
  for (uint8_t byte : command) {
    chk += byte;
  }
  chk = (chk ^ 0xFF) + 1;
  return chk;
}

bool HeishamonProtocol::is_valid_checksum(const std::vector<uint8_t> &data) {
  uint8_t chk = 0;
  for (uint8_t byte : data) {
    chk += byte;
  }
  return (chk == 0);
}

void HeishamonProtocol::push_command_buffer(const std::vector<uint8_t> &command) {
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

void HeishamonProtocol::pop_command_buffer() {
  if (this->cmd_count_ == 0) return;
  
  const CommandBuffer &cmd = this->command_buffer_[this->cmd_start_];
  std::vector<uint8_t> command(cmd.data, cmd.data + cmd.size);
  
  // Send command
  uint8_t checksum = this->calc_checksum(command);
  command.push_back(checksum);
  
  this->uart_->write_array(command);
  this->sending_ = true;
  this->send_command_read_time_ = millis();
  
  this->cmd_start_ = (this->cmd_start_ + 1) % MAXCOMMANDSINBUFFER;
  this->cmd_count_--;
}

}  // namespace heishamon
}  // namespace esphome