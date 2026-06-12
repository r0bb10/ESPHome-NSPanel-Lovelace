#pragma once

#include <string>

#include "esphome/components/uart/uart.h"

namespace esphome {
namespace nspanel_lovelace {

class NextionTransport {
 public:
  void set_uart(uart::UARTDevice *uart) { this->uart_ = uart; }

  void send_command(const std::string &command);

 protected:
  uart::UARTDevice *uart_{nullptr};
};

}  // namespace nspanel_lovelace
}  // namespace esphome
