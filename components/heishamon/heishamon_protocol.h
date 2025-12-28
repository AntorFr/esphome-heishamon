#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include <vector>
#include <functional>

namespace esphome {
namespace heishamon {

// Protocol constants (extracted from original heishamon.h)
static const int DATASIZE = 203;
static const int OPTDATASIZE = 20;
static const int MAXDATASIZE = 255;
static const int PANASONICQUERYSIZE = 110;
static const int SERIALTIMEOUT = 2900;
static const int MAXCOMMANDSINBUFFER = 10;

// Command buffer structure
struct CommandBuffer {
  uint8_t data[20];
  size_t size;
};

/**
 * HeishamonProtocol - Pure protocol communication layer
 * Handles all low-level serial communication with the heat pump
 * Independent of ESPHome component logic
 */
class HeishamonProtocol {
 public:
  explicit HeishamonProtocol(uart::UARTDevice *uart);
  
  // Initialization
  void init();
  
  // Serial communication
  bool read_serial();
  void send_initial_query();
  void send_panasonic_query();
  void send_optional_pcb_query();
  bool send_command(const std::vector<uint8_t> &command);
  
  // Protocol validation
  uint8_t calc_checksum(const std::vector<uint8_t> &command);
  bool is_valid_checksum(const std::vector<uint8_t> &data);
  
  // Data access
  const std::vector<uint8_t> &get_current_data() const { return act_data_; }
  const std::vector<uint8_t> &get_extra_data() const { return act_data_extra_; }
  const std::vector<uint8_t> &get_optional_data() const { return act_opt_data_; }
  
  // State management
  bool is_sending() const { return sending_; }
  void set_listen_only(bool listen_only) { listen_only_ = listen_only; }
  void set_optional_pcb(bool optional_pcb) { optional_pcb_ = optional_pcb; }
  
  // Statistics
  uint32_t get_total_reads() const { return total_reads_; }
  uint32_t get_good_reads() const { return good_reads_; }
  uint32_t get_bad_header_reads() const { return bad_header_reads_; }
  uint32_t get_bad_crc_reads() const { return bad_crc_reads_; }
  uint32_t get_timeout_reads() const { return timeout_reads_; }
  
  // Data notification callback
  void set_data_callback(std::function<void(const std::vector<uint8_t> &, uint8_t)> callback) {
    data_callback_ = callback;
  }
  
  // Process periodic tasks (call from component loop)
  void process_loop();
  
 private:
  // UART reference
  uart::UARTDevice *uart_;
  
  // Protocol state
  bool sending_{false};
  bool listen_only_{false};
  bool optional_pcb_{false};
  bool initial_query_sent_{false};
  bool extra_data_available_{false};
  
  // Timing
  uint32_t send_command_read_time_{0};
  uint32_t last_optional_pcb_time_{0};
  
  // Data buffers
  std::vector<uint8_t> data_buffer_;
  std::vector<uint8_t> act_data_;
  std::vector<uint8_t> act_data_extra_;
  std::vector<uint8_t> act_opt_data_;
  
  // Predefined queries
  std::vector<uint8_t> initial_query_;
  std::vector<uint8_t> panasonic_query_;
  std::vector<uint8_t> optional_pcb_query_;
  
  // Command buffer management
  CommandBuffer command_buffer_[MAXCOMMANDSINBUFFER];
  int cmd_start_{0};
  int cmd_end_{0};
  int cmd_count_{0};
  
  // Statistics
  uint32_t total_reads_{0};
  uint32_t good_reads_{0};
  uint32_t bad_header_reads_{0};
  uint32_t bad_crc_reads_{0};
  uint32_t timeout_reads_{0};
  
  // Data notification callback
  std::function<void(const std::vector<uint8_t> &, uint8_t)> data_callback_;
  
  // Internal methods
  void init_queries();
  void push_command_buffer(const std::vector<uint8_t> &command);
  void pop_command_buffer();
  void check_timeouts();
  void process_commands();
  
  // Data type constants for callback
  static const uint8_t DATA_TYPE_NORMAL = 0x10;
  static const uint8_t DATA_TYPE_EXTRA = 0x21;
  static const uint8_t DATA_TYPE_OPTIONAL = 0xF1;
};

}  // namespace heishamon
}  // namespace esphome