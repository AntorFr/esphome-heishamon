#pragma once

#include <map>
#include <string>
#include <functional>
#include <vector>

namespace esphome {
namespace heishamon {

/**
 * HeishamonCallbackManager - Central callback management
 * 
 * Manages all callbacks from protocol layer to ESPHome components
 * Provides a clean interface for component registration and data distribution
 */
class HeishamonCallbackManager {
 public:
  // Callback registration
  void register_sensor_callback(const std::string &topic, std::function<void(float)> callback);
  void register_binary_sensor_callback(const std::string &topic, std::function<void(bool)> callback);
  void register_text_sensor_callback(const std::string &topic, std::function<void(const std::string&)> callback);
  void register_switch_callback(const std::string &topic, std::function<void(bool)> callback);
  void register_select_callback(const std::string &topic, std::function<void(const std::string&)> callback);
  
  // Check if callback exists (to skip decoding if not needed)
  bool has_sensor_callback(const std::string &topic) const { return sensor_callbacks_.count(topic) > 0; }
  bool has_binary_sensor_callback(const std::string &topic) const { return binary_sensor_callbacks_.count(topic) > 0; }
  bool has_switch_callback(const std::string &topic) const { return switch_callbacks_.count(topic) > 0; }
  bool has_select_callback(const std::string &topic) const { return select_callbacks_.count(topic) > 0; }
  bool has_text_sensor_callback(const std::string &topic) const { return text_sensor_callbacks_.count(topic) > 0; }
  
  // Data notification from protocol layer
  void notify_sensor_value(const std::string &topic, float value);
  void notify_binary_sensor_value(const std::string &topic, bool value);
  void notify_text_sensor_value(const std::string &topic, const std::string &value);
  void notify_switch_value(const std::string &topic, bool value);
  void notify_select_value(const std::string &topic, const std::string &value);
  
  // Batch notification for efficient processing
    void notify_all_sensors(const std::map<std::string, float> &sensor_values,
                          const std::map<std::string, bool> &binary_values,
                          const std::map<std::string, std::string> &text_values,
                          const std::map<std::string, bool> &switch_values,
                          const std::map<std::string, std::string> &select_values = {});
  
  // Utility functions
  void clear_all_callbacks();
  size_t get_sensor_callback_count() const { return sensor_callbacks_.size(); }
  size_t get_binary_sensor_callback_count() const { return binary_sensor_callbacks_.size(); }
  size_t get_text_sensor_callback_count() const { return text_sensor_callbacks_.size(); }
  size_t get_switch_callback_count() const { return switch_callbacks_.size(); }
  size_t get_select_callback_count() const { return select_callbacks_.size(); }
  
  // Debug information
  std::vector<std::string> get_registered_sensor_topics() const;
  std::vector<std::string> get_registered_binary_sensor_topics() const;
  std::vector<std::string> get_registered_text_sensor_topics() const;
  std::vector<std::string> get_registered_switch_topics() const;
  std::vector<std::string> get_registered_select_topics() const;
  
 private:
  // Callback storage - using vector to support multiple callbacks per topic
  std::map<std::string, std::vector<std::function<void(float)>>> sensor_callbacks_;
  std::map<std::string, std::vector<std::function<void(bool)>>> binary_sensor_callbacks_;
  std::map<std::string, std::vector<std::function<void(const std::string&)>>> text_sensor_callbacks_;
  std::map<std::string, std::vector<std::function<void(bool)>>> switch_callbacks_;
  std::map<std::string, std::vector<std::function<void(const std::string&)>>> select_callbacks_;
  
  // Value cache to avoid publishing unchanged values (reduces API load)
  std::map<std::string, float> sensor_cache_;
  std::map<std::string, bool> binary_sensor_cache_;
  std::map<std::string, std::string> text_sensor_cache_;
  std::map<std::string, bool> switch_cache_;
  std::map<std::string, std::string> select_cache_;
  
  // Statistics for debugging
  uint32_t total_sensor_notifications_{0};
  uint32_t total_binary_sensor_notifications_{0};
  uint32_t total_text_sensor_notifications_{0};
  uint32_t total_switch_notifications_{0};
  uint32_t total_select_notifications_{0};
  uint32_t skipped_unchanged_{0};
  
 public:
  // Statistics getters
  uint32_t get_total_sensor_notifications() const { return total_sensor_notifications_; }
  uint32_t get_total_binary_sensor_notifications() const { return total_binary_sensor_notifications_; }
  uint32_t get_total_text_sensor_notifications() const { return total_text_sensor_notifications_; }
  uint32_t get_total_switch_notifications() const { return total_switch_notifications_; }
  uint32_t get_total_select_notifications() const { return total_select_notifications_; }
  uint32_t get_skipped_unchanged() const { return skipped_unchanged_; }
};

}  // namespace heishamon
}  // namespace esphome