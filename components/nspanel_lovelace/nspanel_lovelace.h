#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "display_command_queue.h"
#include "nextion_transport.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace nspanel_lovelace {

class NSPanelLovelace : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_model(const std::string &model) { this->model_ = model; }
  void set_sleep_timeout(uint16_t sleep_timeout) { this->sleep_timeout_ = sleep_timeout; }
  void set_active_brightness(uint8_t active_brightness) { this->active_brightness_ = active_brightness; }
  void set_screensaver_brightness(uint8_t screensaver_brightness) { this->screensaver_brightness_ = screensaver_brightness; }
  void set_screensaver_enabled(bool screensaver_enabled) { this->screensaver_enabled_ = screensaver_enabled; }
  void set_time(time::RealTimeClock *time) { this->time_ = time; }
  void set_time_format(const std::string &time_format) { this->time_format_ = time_format; }
  void set_date_format(const std::string &date_format) { this->date_format_ = date_format; }
  void send_display_command(std::string command) { this->command_queue_.push(std::move(command)); }

 protected:
  void apply_display_settings_();
  void show_screensaver_();
  void update_datetime_();

  std::string model_{"eu"};
  uint16_t sleep_timeout_{20};
  uint8_t active_brightness_{100};
  uint8_t screensaver_brightness_{20};
  bool screensaver_enabled_{false};
  time::RealTimeClock *time_{nullptr};
  std::string time_format_{"%H:%M"};
  std::string date_format_{"%A, %d. %B %Y"};
  uint32_t last_datetime_update_{0};
  NextionTransport transport_;
  DisplayCommandQueue command_queue_;
};

}  // namespace nspanel_lovelace
}  // namespace esphome
