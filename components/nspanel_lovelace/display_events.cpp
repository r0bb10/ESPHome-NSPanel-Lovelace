#include "nspanel_lovelace.h"

#include <cstdlib>

#include "helpers.h"
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
  } else if (parts[1] == "pageOpenDetail") {
    this->handle_page_open_detail_event_(parts);
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
  this->close_popup_();
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

  if (this->handle_detail_action_(internal_id, button_type, value)) {
    return;
  }

  this->handle_entity_action_(internal_id, button_type, value);
}

void NSPanelLovelace::handle_page_open_detail_event_(const std::vector<std::string> &parts) {
  if (parts.size() < 4) {
    return;
  }
  this->show_popup_(parts[2], parts[3]);
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
    this->call_ha_service_(domain, value == "1" ? "turn_on" : "turn_off", {{"entity_id", entity_id}});
    return;
  }

  if (button_type == "number-set") {
    if (domain == "fan") {
      auto entity = this->find_card_entity_(entity_id);
      float step = 1.0f;
      if (entity != nullptr && entity->attributes.count("percentage_step")) {
        parse_float_(entity->attributes.at("percentage_step"), step);
      }
      if (step > 100.0f) step = 100.0f;
      float slider_value = 0.0f;
      if (!parse_float_(value, slider_value)) return;
      auto pct = slider_value * step;
      if (pct > 100.0f) pct = 100.0f;
      char buf[16];
      snprintf(buf, sizeof(buf), "%.6f", pct);
      this->call_ha_service_("fan", "set_percentage", {{"entity_id", entity_id}, {"percentage", buf}});
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

bool NSPanelLovelace::handle_detail_action_(const std::string &entity_id, const std::string &button_type,
                                            const std::string &value) {
  const auto domain = entity_domain_(entity_id);
  if (domain.empty() || entity_id.find('.') == std::string::npos) {
    return false;
  }

  if (button_type == "brightnessSlider") {
    int brightness = 0;
    if (!parse_int_(value, brightness)) return true;
    auto ha_brightness = static_cast<int>(scale_value(brightness, {0, 100}, {0, 255}));
    this->call_ha_service_(domain, "turn_on", {{"entity_id", entity_id}, {"brightness", std::to_string(ha_brightness)}});
    return true;
  }

  if (button_type == "colorTempSlider") {
    int slider = 0;
    if (!parse_int_(value, slider)) return true;
    auto entity = this->find_card_entity_(entity_id);
    int min_mireds = 153;
    int max_mireds = 500;
    if (entity != nullptr) {
      if (entity->attributes.count("min_mireds")) parse_int_(entity->attributes.at("min_mireds"), min_mireds);
      if (entity->attributes.count("max_mireds")) parse_int_(entity->attributes.at("max_mireds"), max_mireds);
    }
    if (min_mireds >= max_mireds || min_mireds <= 0 || max_mireds <= 0) {
      min_mireds = 153;
      max_mireds = 500;
    }
    auto min_kelvin = 1000000.0 / max_mireds;
    auto max_kelvin = 1000000.0 / min_mireds;
    auto kelvin = scale_value(slider, {0, 100}, {max_kelvin, min_kelvin});
    auto mireds = static_cast<int>(1000000.0 / kelvin);
    this->call_ha_service_(domain, "turn_on", {{"entity_id", entity_id}, {"color_temp", std::to_string(mireds)}});
    return true;
  }

  if (button_type == "colorWheel") {
    auto tokens = split_(value, '|');
    if (tokens.size() != 3) return true;
    double x = 0, y = 0;
    float wh = 0;
    if (!parse_float_(tokens[0], x) || !parse_float_(tokens[1], y) || !parse_float_(tokens[2], wh)) return true;
    auto rgb = xy_to_rgb(x, y, wh);
    this->call_ha_service_(domain, "turn_on", {{"entity_id", entity_id}, {"rgb_color", join_rgb_(rgb)}});
    return true;
  }

  if (button_type == "positionSlider") {
    this->call_ha_service_(domain, "set_cover_position", {{"entity_id", entity_id}, {"position", value}});
    return true;
  }

  if (button_type == "tiltOpen") {
    this->call_ha_service_(domain, "open_cover_tilt", {{"entity_id", entity_id}});
    return true;
  }
  if (button_type == "tiltStop") {
    this->call_ha_service_(domain, "stop_cover_tilt", {{"entity_id", entity_id}});
    return true;
  }
  if (button_type == "tiltClose") {
    this->call_ha_service_(domain, "close_cover_tilt", {{"entity_id", entity_id}});
    return true;
  }
  if (button_type == "tiltSlider") {
    this->call_ha_service_(domain, "set_cover_tilt_position", {{"entity_id", entity_id}, {"tilt_position", value}});
    return true;
  }

  if (button_type == "modePresetModes" || button_type == "mode-preset_modes") {
    auto entity = this->find_card_entity_(entity_id);
    if (entity == nullptr || !entity->attributes.count("preset_modes")) return true;
    auto modes = split_(entity->attributes.at("preset_modes"), ',');
    int index = 0;
    if (!parse_int_(value, index) || index < 0 || static_cast<size_t>(index) >= modes.size()) return true;
    this->call_ha_service_(domain, "set_preset_mode", {{"entity_id", entity_id}, {"preset_mode", modes[index]}});
    return true;
  }

  if (button_type == "modeSwingModes" || button_type == "mode-swing_modes") {
    auto entity = this->find_card_entity_(entity_id);
    if (entity == nullptr || !entity->attributes.count("swing_modes")) return true;
    auto modes = split_(entity->attributes.at("swing_modes"), ',');
    int index = 0;
    if (!parse_int_(value, index) || index < 0 || static_cast<size_t>(index) >= modes.size()) return true;
    this->call_ha_service_(domain, "set_swing_mode", {{"entity_id", entity_id}, {"swing_mode", modes[index]}});
    return true;
  }

  if (button_type == "modeFanModes" || button_type == "mode-fan_modes") {
    auto entity = this->find_card_entity_(entity_id);
    if (entity == nullptr || !entity->attributes.count("fan_modes")) return true;
    auto modes = split_(entity->attributes.at("fan_modes"), ',');
    int index = 0;
    if (!parse_int_(value, index) || index < 0 || static_cast<size_t>(index) >= modes.size()) return true;
    this->call_ha_service_(domain, "set_fan_mode", {{"entity_id", entity_id}, {"fan_mode", modes[index]}});
    return true;
  }

  if (button_type == "tempUpd") {
    int raw = 0;
    if (!parse_int_(value, raw)) return true;
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", raw * 0.1f);
    this->call_ha_service_(domain, "set_temperature", {{"entity_id", entity_id}, {"temperature", buf}});
    return true;
  }

  if (button_type == "tempUpdHighLow") {
    auto temps = split_(value, '|');
    if (temps.size() != 2) return true;
    int high = 0, low = 0;
    if (!parse_int_(temps[0], high) || !parse_int_(temps[1], low)) return true;
    char high_buf[16], low_buf[16];
    snprintf(high_buf, sizeof(high_buf), "%.1f", high * 0.1f);
    snprintf(low_buf, sizeof(low_buf), "%.1f", low * 0.1f);
    this->call_ha_service_(domain, "set_temperature",
                           {{"entity_id", entity_id}, {"target_temp_high", high_buf}, {"target_temp_low", low_buf}});
    return true;
  }

  if (button_type == "hvac_action") {
    this->call_ha_service_(domain, "set_hvac_mode", {{"entity_id", entity_id}, {"hvac_mode", value}});
    return true;
  }

  if (button_type == "modeInputSelect" || button_type == "modeSelect") {
    auto entity = this->find_card_entity_(entity_id);
    if (entity == nullptr || !entity->attributes.count("options")) return true;
    auto options = split_(entity->attributes.at("options"), ',');
    int index = 0;
    if (!parse_int_(value, index) || index < 0 || static_cast<size_t>(index) >= options.size()) return true;
    this->call_ha_service_(domain, "select_option", {{"entity_id", entity_id}, {"option", options[index]}});
    return true;
  }

  return false;
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
