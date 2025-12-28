#include "heishamon_callbacks.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace heishamon {

static const char *const TAG = "heishamon.callbacks";

void HeishamonCallbackManager::register_sensor_callback(const std::string &topic, std::function<void(float)> callback) {
  ESP_LOGD(TAG, "Registering sensor callback for topic: %s", topic.c_str());
  this->sensor_callbacks_[topic].push_back(callback);
  ESP_LOGD(TAG, "Total sensor topics registered: %d, callbacks for %s: %d", 
           this->sensor_callbacks_.size(), topic.c_str(), this->sensor_callbacks_[topic].size());
}

void HeishamonCallbackManager::register_binary_sensor_callback(const std::string &topic, std::function<void(bool)> callback) {
  ESP_LOGD(TAG, "Registering binary sensor callback for topic: %s", topic.c_str());
  this->binary_sensor_callbacks_[topic].push_back(callback);
  ESP_LOGD(TAG, "Total binary sensor topics registered: %d", this->binary_sensor_callbacks_.size());
}

void HeishamonCallbackManager::register_text_sensor_callback(const std::string &topic, std::function<void(const std::string&)> callback) {
  ESP_LOGD(TAG, "Registering text sensor callback for topic: %s", topic.c_str());
  this->text_sensor_callbacks_[topic].push_back(callback);
  ESP_LOGD(TAG, "Total text sensor topics registered: %d", this->text_sensor_callbacks_.size());
}

void HeishamonCallbackManager::register_switch_callback(const std::string &topic, std::function<void(bool)> callback) {
  ESP_LOGD(TAG, "Registering switch callback for topic: %s", topic.c_str());
  this->switch_callbacks_[topic].push_back(callback);
  ESP_LOGD(TAG, "Total switch topics registered: %d", this->switch_callbacks_.size());
}

void HeishamonCallbackManager::notify_sensor_value(const std::string &topic, float value) {
  auto it = this->sensor_callbacks_.find(topic);
  if (it != this->sensor_callbacks_.end()) {
    // Check cache - only publish if value changed (with small tolerance for floats)
    auto cache_it = this->sensor_cache_.find(topic);
    if (cache_it != this->sensor_cache_.end() && std::abs(cache_it->second - value) < 0.01f) {
      this->skipped_unchanged_++;
      return;  // Value unchanged, skip notification
    }
    this->sensor_cache_[topic] = value;
    ESP_LOGV(TAG, "Notifying %d callbacks for sensor %s with value %.2f", it->second.size(), topic.c_str(), value);
    for (auto &callback : it->second) {
      callback(value);
    }
    this->total_sensor_notifications_++;
  }
}

void HeishamonCallbackManager::notify_binary_sensor_value(const std::string &topic, bool value) {
  auto it = this->binary_sensor_callbacks_.find(topic);
  if (it != this->binary_sensor_callbacks_.end()) {
    // Check cache - only publish if value changed
    auto cache_it = this->binary_sensor_cache_.find(topic);
    if (cache_it != this->binary_sensor_cache_.end() && cache_it->second == value) {
      this->skipped_unchanged_++;
      return;  // Value unchanged, skip notification
    }
    this->binary_sensor_cache_[topic] = value;
    ESP_LOGV(TAG, "Notifying %d callbacks for binary sensor %s with value %s", it->second.size(), topic.c_str(), value ? "true" : "false");
    for (auto &callback : it->second) {
      callback(value);
    }
    this->total_binary_sensor_notifications_++;
  }
}

void HeishamonCallbackManager::notify_text_sensor_value(const std::string &topic, const std::string &value) {
  auto it = this->text_sensor_callbacks_.find(topic);
  if (it != this->text_sensor_callbacks_.end()) {
    // Check cache - only publish if value changed
    auto cache_it = this->text_sensor_cache_.find(topic);
    if (cache_it != this->text_sensor_cache_.end() && cache_it->second == value) {
      this->skipped_unchanged_++;
      return;  // Value unchanged, skip notification
    }
    this->text_sensor_cache_[topic] = value;
    ESP_LOGD(TAG, "Notifying %d callbacks for text sensor %s with value %s", it->second.size(), topic.c_str(), value.c_str());
    for (auto &callback : it->second) {
      callback(value);
    }
    this->total_text_sensor_notifications_++;
  }
}

void HeishamonCallbackManager::notify_switch_value(const std::string &topic, bool value) {
  auto it = this->switch_callbacks_.find(topic);
  if (it != this->switch_callbacks_.end()) {
    // Check cache - only publish if value changed
    auto cache_it = this->switch_cache_.find(topic);
    if (cache_it != this->switch_cache_.end() && cache_it->second == value) {
      this->skipped_unchanged_++;
      return;  // Value unchanged, skip notification
    }
    this->switch_cache_[topic] = value;
    ESP_LOGV(TAG, "Notifying %d callbacks for switch %s with value %s", it->second.size(), topic.c_str(), value ? "ON" : "OFF");
    for (auto &callback : it->second) {
      callback(value);
    }
    this->total_switch_notifications_++;
  }
}

void HeishamonCallbackManager::register_select_callback(const std::string &topic, std::function<void(const std::string&)> callback) {
  ESP_LOGD(TAG, "Registering select callback for topic: %s", topic.c_str());
  this->select_callbacks_[topic].push_back(std::move(callback));
}

void HeishamonCallbackManager::notify_select_value(const std::string &topic, const std::string &value) {
  auto it = this->select_callbacks_.find(topic);
  if (it != this->select_callbacks_.end()) {
    // Check cache - only publish if value changed
    auto cache_it = this->select_cache_.find(topic);
    if (cache_it != this->select_cache_.end() && cache_it->second == value) {
      this->skipped_unchanged_++;
      return;  // Value unchanged, skip notification
    }
    this->select_cache_[topic] = value;
    ESP_LOGV(TAG, "Notifying %d callbacks for select %s with value %s", it->second.size(), topic.c_str(), value.c_str());
    for (auto &callback : it->second) {
      callback(value);
    }
    this->total_select_notifications_++;
  }
}

void HeishamonCallbackManager::notify_all_sensors(const std::map<std::string, float> &sensor_values,
                                                 const std::map<std::string, bool> &binary_values,
                                                 const std::map<std::string, std::string> &text_values,
                                                 const std::map<std::string, bool> &switch_values,
                                                 const std::map<std::string, std::string> &select_values) {
  ESP_LOGD(TAG, "Batch notification: %d sensors, %d binary sensors, %d text sensors, %d switches, %d selects", 
           sensor_values.size(), binary_values.size(), text_values.size(), switch_values.size(), select_values.size());
  
  // Process sensor values
  for (const auto &pair : sensor_values) {
    this->notify_sensor_value(pair.first, pair.second);
  }
  
  // Process binary sensor values
  for (const auto &pair : binary_values) {
    this->notify_binary_sensor_value(pair.first, pair.second);
  }
  
  // Process text sensor values
  for (const auto &pair : text_values) {
    this->notify_text_sensor_value(pair.first, pair.second);
  }
  
  // Process switch values
  for (const auto &pair : switch_values) {
    this->notify_switch_value(pair.first, pair.second);
  }
  
  // Process select values
  for (const auto &pair : select_values) {
    this->notify_select_value(pair.first, pair.second);
  }
}

void HeishamonCallbackManager::clear_all_callbacks() {
  ESP_LOGD(TAG, "Clearing all callbacks");
  this->sensor_callbacks_.clear();
  this->binary_sensor_callbacks_.clear();
  this->text_sensor_callbacks_.clear();
  this->switch_callbacks_.clear();
  this->select_callbacks_.clear();
}

std::vector<std::string> HeishamonCallbackManager::get_registered_sensor_topics() const {
  std::vector<std::string> topics;
  topics.reserve(this->sensor_callbacks_.size());
  for (const auto &pair : this->sensor_callbacks_) {
    topics.push_back(pair.first);
  }
  return topics;
}

std::vector<std::string> HeishamonCallbackManager::get_registered_binary_sensor_topics() const {
  std::vector<std::string> topics;
  topics.reserve(this->binary_sensor_callbacks_.size());
  for (const auto &pair : this->binary_sensor_callbacks_) {
    topics.push_back(pair.first);
  }
  return topics;
}

std::vector<std::string> HeishamonCallbackManager::get_registered_text_sensor_topics() const {
  std::vector<std::string> topics;
  topics.reserve(this->text_sensor_callbacks_.size());
  for (const auto &pair : this->text_sensor_callbacks_) {
    topics.push_back(pair.first);
  }
  return topics;
}

std::vector<std::string> HeishamonCallbackManager::get_registered_switch_topics() const {
  std::vector<std::string> topics;
  topics.reserve(this->switch_callbacks_.size());
  for (const auto &pair : this->switch_callbacks_) {
    topics.push_back(pair.first);
  }
  return topics;
}

std::vector<std::string> HeishamonCallbackManager::get_registered_select_topics() const {
  std::vector<std::string> topics;
  topics.reserve(this->select_callbacks_.size());
  for (const auto &pair : this->select_callbacks_) {
    topics.push_back(pair.first);
  }
  return topics;
}

}  // namespace heishamon
}  // namespace esphome