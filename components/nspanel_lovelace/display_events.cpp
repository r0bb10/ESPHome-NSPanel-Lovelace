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
    if (internal_id.rfind("navigate.uuid.", 0) == 0 || internal_id == "navPrev" ||
        internal_id == "navNext" || internal_id == "navUp") {
      return;
    }
  }

  this->handle_entity_action_(internal_id, button_type, value);
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

void NSPanelLovelace::handle_entity_action_(const std::string &entity_id, const std::string &button_type,
                                            const std::string &value) {
  const auto domain = entity_domain_(entity_id);
  if (domain.empty() || entity_id.find('.') == std::string::npos) {
    return;
  }

  if (button_type == "OnOff") {
    this->call_ha_service_(domain, value == "1" ? "turn_off" : "turn_on", {{"entity_id", entity_id}});
    return;
  }

  if (button_type == "number-set") {
    if (domain == "fan") {
      this->call_ha_service_("fan", "set_percentage", {{"entity_id", entity_id}, {"percentage", value}});
    } else {
      this->call_ha_service_(domain, "set_value", {{"entity_id", entity_id}, {"value", value}});
    }
    return;
  }

  if (button_type == "up") {
    this->call_ha_service_(domain, "open_cover", {{"entity_id", entity_id}});
    return;
  }
  if (button_type == "stop") {
    this->call_ha_service_(domain, "stop_cover", {{"entity_id", entity_id}});
    return;
  }
  if (button_type == "down") {
    this->call_ha_service_(domain, "close_cover", {{"entity_id", entity_id}});
    return;
  }

  if (button_type == "button") {
    if (domain == "scene" || domain == "script") {
      this->call_ha_service_(domain, "turn_on", {{"entity_id", entity_id}});
    } else if (domain == "button" || domain == "input_button") {
      this->call_ha_service_(domain, "press", {{"entity_id", entity_id}});
    } else if (domain == "light" || domain == "switch" || domain == "input_boolean" ||
               domain == "automation" || domain == "fan") {
      this->call_ha_service_(domain, "toggle", {{"entity_id", entity_id}});
    }
    return;
  }
}

void NSPanelLovelace::call_ha_service_(const std::string &service, const std::string &entity_id) {
  std::map<std::string, std::string> data;
  data.emplace("entity_id", entity_id);
  this->call_ha_service_(service, data);
}

void NSPanelLovelace::call_ha_service_(const std::string &service, const std::map<std::string, std::string> &data) {
  ESP_LOGD(TAG, "Call HA: %s", service.c_str());
  this->call_homeassistant_service(service, data);
}

void NSPanelLovelace::call_ha_service_(const std::string &domain, const std::string &service,
                                       const std::map<std::string, std::string> &data) {
  std::string full_service = domain;
  full_service.append(1, '.').append(service);
  this->call_ha_service_(full_service, data);
}

}  // namespace nspanel_lovelace
}  // namespace esphome
