#pragma once

#include <string>

#include "nextion_transport.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace nspanel_lovelace {

class NSPanelLovelace : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void dump_config() override;

  void set_model(const std::string &model) { this->model_ = model; }
  void send_display_command(const std::string &command) { this->transport_.send_command(command); }

 protected:
  std::string model_{"eu"};
  NextionTransport transport_;
};

}  // namespace nspanel_lovelace
}  // namespace esphome
