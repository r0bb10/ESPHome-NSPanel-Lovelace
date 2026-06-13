#include "nspanel_lovelace.h"

#include <cstdlib>

#include "esphome/core/log.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace.events";

void NSPanelLovelace::process_display_messages_() {
  std::string message;
  while (this->transport_.read_payload(message)) {
    this->process_display_message_(message);
  }
}

void NSPanelLovelace::process_display_message_(const std::string &message) {
  ESP_LOGD(TAG, "TFT event: %s", message.c_str());
  const auto parts = split_(message, ',');
  if (parts.size() < 2 || parts[0] != "event") {
    return;
  }

  if (parts[1] == "startup") {
    this->handle_startup_event_(parts);
  } else if (parts[1] == "sleepReached") {
    this->handle_sleep_reached_event_();
  } else if (parts[1] == "buttonPress2") {
    this->handle_button_press_event_(parts);
  }
}

void NSPanelLovelace::handle_startup_event_(const std::vector<std::string> &parts) {
  if (parts.size() > 2) {
    this->tft_version_ = parts[2];
  }
  if (parts.size() > 3) {
    this->tft_model_ = parts[3];
  }
  this->display_started_ = true;
  this->apply_display_settings_();
  if (this->screensaver_enabled_) {
    this->show_screensaver_from_event_();
  } else if (!this->cards_.empty()) {
    this->show_card_(this->current_card_);
  }
}

void NSPanelLovelace::handle_sleep_reached_event_() {
  if (this->screensaver_enabled_) {
    this->show_screensaver_from_event_();
  }
}

void NSPanelLovelace::handle_button_press_event_(const std::vector<std::string> &parts) {
  if (parts.size() < 4) {
    return;
  }

  const auto &internal_id = parts[2];
  const auto &button_type = parts[3];
  const auto value = parts.size() > 4 ? parts[4] : "";

  if (internal_id == "screensaver" && button_type == "bExit") {
    const bool single_tap = value.empty() || value == "1";
    if ((single_tap || value >= "2") && !this->cards_.empty()) {
      this->show_card_(this->current_card_);
    }
    return;
  }

  if (button_type == "button") {
    this->handle_navigation_button_(internal_id);
  }
}

void NSPanelLovelace::handle_navigation_button_(const std::string &internal_id) {
  if (this->cards_.empty()) {
    return;
  }
  if (internal_id == "navPrev") {
    this->show_card_(this->current_card_ == 0 ? this->cards_.size() - 1 : this->current_card_ - 1);
    return;
  }
  if (internal_id == "navNext") {
    this->show_card_((this->current_card_ + 1) % this->cards_.size());
    return;
  }
  if (internal_id == "navUp") {
    this->render_current_card_();
    return;
  }
  const std::string prefix{"navigate.uuid."};
  if (internal_id.rfind(prefix, 0) == 0) {
    const auto index_text = internal_id.substr(prefix.size());
    char *end{nullptr};
    const auto index = strtoul(index_text.c_str(), &end, 10);
    if (end != index_text.c_str() && *end == '\0' && index < this->cards_.size()) {
      this->show_card_(index);
    }
  }
}

}  // namespace nspanel_lovelace
}  // namespace esphome
