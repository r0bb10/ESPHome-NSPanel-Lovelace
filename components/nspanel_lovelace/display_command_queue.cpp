#include "display_command_queue.h"

#include <utility>

namespace esphome {
namespace nspanel_lovelace {

void DisplayCommandQueue::push(std::string command) {
  this->commands_.push(std::move(command));
}

void DisplayCommandQueue::process_one(NextionTransport &transport) {
  if (this->commands_.empty()) {
    return;
  }

  transport.send_command(this->commands_.front());
  this->commands_.pop();
}

}  // namespace nspanel_lovelace
}  // namespace esphome
