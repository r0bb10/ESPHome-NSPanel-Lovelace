#include "nspanel_lovelace.h"

#include "esphome/core/log.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace";

void NSPanelLovelace::setup() {
  this->transport_.set_uart(this);
}

void NSPanelLovelace::dump_config() {
  ESP_LOGCONFIG(TAG, "NSPanel Lovelace Native v2");
  ESP_LOGCONFIG(TAG, "  Model: %s", this->model_.c_str());
}

}  // namespace nspanel_lovelace
}  // namespace esphome
