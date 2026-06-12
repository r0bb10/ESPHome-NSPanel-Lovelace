#include "nextion_transport.h"

namespace esphome {
namespace nspanel_lovelace {

void NextionTransport::send_command(const std::string &command) {
  if (this->uart_ == nullptr) {
    return;
  }

  this->uart_->write_str(command.c_str());
  this->uart_->write_byte(0xFF);
  this->uart_->write_byte(0xFF);
  this->uart_->write_byte(0xFF);
}

}  // namespace nspanel_lovelace
}  // namespace esphome
