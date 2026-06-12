#include "nspanel_lovelace.h"

#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace";

void NSPanelLovelace::setup() {
  this->transport_.set_uart(this);
  this->apply_display_settings_();
  this->show_screensaver_();
  this->update_datetime_();
}

void NSPanelLovelace::loop() {
  const uint32_t now = millis();
  if (now - this->last_datetime_update_ >= 60000) {
    this->update_datetime_();
  }

  this->command_queue_.process_one(this->transport_);
}

void NSPanelLovelace::dump_config() {
  ESP_LOGCONFIG(TAG, "NSPanel Lovelace Native v2");
  ESP_LOGCONFIG(TAG, "  Model: %s", this->model_.c_str());
  ESP_LOGCONFIG(TAG, "  Sleep Timeout: %us", this->sleep_timeout_);
  ESP_LOGCONFIG(TAG, "  Active Brightness: %u", this->active_brightness_);
  ESP_LOGCONFIG(TAG, "  Screensaver Brightness: %u", this->screensaver_brightness_);
  ESP_LOGCONFIG(TAG, "  Screensaver: %s", YESNO(this->screensaver_enabled_));
  ESP_LOGCONFIG(TAG, "  Time Source: %s", this->time_ == nullptr ? "none" : "configured");
}

void NSPanelLovelace::apply_display_settings_() {
  this->send_display_command("timeout~" + std::to_string(this->sleep_timeout_));
  this->send_display_command("dimmode~" + std::to_string(this->screensaver_brightness_) + "~" +
                             std::to_string(this->active_brightness_));
}

void NSPanelLovelace::show_screensaver_() {
  if (!this->screensaver_enabled_) {
    return;
  }

  this->send_display_command("pageType~screensaver");
}

void NSPanelLovelace::update_datetime_() {
  this->last_datetime_update_ = millis();
  if (this->time_ == nullptr) {
    return;
  }

  auto now = this->time_->now();
  if (!now.is_valid()) {
    return;
  }

  this->send_display_command("time~" + now.strftime(this->time_format_));
  this->send_display_command("date~" + now.strftime(this->date_format_));
}

}  // namespace nspanel_lovelace
}  // namespace esphome
