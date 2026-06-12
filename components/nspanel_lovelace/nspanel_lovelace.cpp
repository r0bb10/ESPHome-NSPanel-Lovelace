#include "nspanel_lovelace.h"

#include "esphome/core/log.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace";

void NSPanelLovelace::setup() {
  this->transport_.set_uart(this);
  this->apply_display_settings_();
}

void NSPanelLovelace::loop() {
  this->command_queue_.process_one(this->transport_);
}

void NSPanelLovelace::dump_config() {
  ESP_LOGCONFIG(TAG, "NSPanel Lovelace Native v2");
  ESP_LOGCONFIG(TAG, "  Model: %s", this->model_.c_str());
  ESP_LOGCONFIG(TAG, "  Sleep Timeout: %us", this->sleep_timeout_);
  ESP_LOGCONFIG(TAG, "  Active Brightness: %u", this->active_brightness_);
  ESP_LOGCONFIG(TAG, "  Screensaver Brightness: %u", this->screensaver_brightness_);
}

void NSPanelLovelace::apply_display_settings_() {
  this->send_display_command("timeout~" + std::to_string(this->sleep_timeout_));
  this->send_display_command("dimmode~" + std::to_string(this->screensaver_brightness_) + "~" +
                             std::to_string(this->active_brightness_));
}

}  // namespace nspanel_lovelace
}  // namespace esphome
