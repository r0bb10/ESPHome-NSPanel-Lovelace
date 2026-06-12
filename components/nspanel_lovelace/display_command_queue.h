#pragma once

#include <queue>
#include <string>

#include "nextion_transport.h"

namespace esphome {
namespace nspanel_lovelace {

class DisplayCommandQueue {
 public:
  void push(std::string command);
  bool empty() const { return this->commands_.empty(); }
  void process_one(NextionTransport &transport);

 protected:
  std::queue<std::string> commands_;
};

}  // namespace nspanel_lovelace
}  // namespace esphome
