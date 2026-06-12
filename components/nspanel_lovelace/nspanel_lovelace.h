#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "display_command_queue.h"
#include "nextion_transport.h"
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
  void send_display_command(std::string command) { this->command_queue_.push(std::move(command)); }

 protected:
  void apply_display_settings_();

  std::string model_{"eu"};
  uint16_t sleep_timeout_{20};
  uint8_t active_brightness_{100};
  uint8_t screensaver_brightness_{20};
  NextionTransport transport_;
  DisplayCommandQueue command_queue_;
};

}  // namespace nspanel_lovelace
}  // namespace esphome
