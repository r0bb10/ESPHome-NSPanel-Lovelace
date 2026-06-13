#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "esphome/components/uart/uart.h"

namespace esphome {
namespace nspanel_lovelace {

class NextionTransport {
 public:
  void set_uart(uart::UARTDevice *uart) { this->uart_ = uart; }

  void send_command(const std::string &command);
  bool read_payload(std::string &payload);

 protected:
  uart::UARTDevice *uart_{nullptr};
  std::vector<uint8_t> rx_buffer_;
};

}  // namespace nspanel_lovelace
}  // namespace esphome
