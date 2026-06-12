#pragma once

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
  void send_display_command(std::string command) { this->command_queue_.push(std::move(command)); }

 protected:
  std::string model_{"eu"};
  NextionTransport transport_;
  DisplayCommandQueue command_queue_;
};

}  // namespace nspanel_lovelace
}  // namespace esphome
