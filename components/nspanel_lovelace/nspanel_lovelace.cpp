#include "nspanel_lovelace.h"

#include <algorithm>
#include <array>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace";

void replace_first(std::string &value, const std::string &search, const std::string &replacement) {
  const size_t pos = value.find(search);
  if (pos != std::string::npos) {
    value.replace(pos, search.size(), replacement);
  }
}

void NSPanelLovelace::setup() {
  this->transport_.set_uart(this);
  this->apply_display_settings_();
  this->subscribe_screensaver_weather_();
  this->subscribe_screensaver_extra_entity_();
  this->subscribe_screensaver_status_icons_();
  this->subscribe_card_entities_();
  if (this->screensaver_enabled_) {
    this->show_screensaver_();
    this->update_datetime_();
    this->render_screensaver_entities_();
    this->render_screensaver_status_icons_();
  } else if (!this->cards_.empty()) {
    this->show_card_(0);
  }
}

void NSPanelLovelace::loop() {
  this->process_display_messages_();

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
  ESP_LOGCONFIG(TAG, "  Language: %s", this->language_.c_str());
  ESP_LOGCONFIG(TAG, "  Screensaver: %s", YESNO(this->screensaver_enabled_));
  ESP_LOGCONFIG(TAG, "  Time Source: %s", this->time_ == nullptr ? "none" : "configured");
  ESP_LOGCONFIG(TAG, "  Screensaver Weather: %s", this->screensaver_weather_.enabled ? this->screensaver_weather_.entity_id.c_str() : "none");
  ESP_LOGCONFIG(TAG, "  Screensaver Extra Entity: %s", this->screensaver_extra_entity_.enabled ? this->screensaver_extra_entity_.entity_id.c_str() : "none");
  ESP_LOGCONFIG(TAG, "  Status Icon Left: %s", this->screensaver_status_icon_left_.enabled ? this->screensaver_status_icon_left_.entity_id.c_str() : "none");
  ESP_LOGCONFIG(TAG, "  Status Icon Right: %s", this->screensaver_status_icon_right_.enabled ? this->screensaver_status_icon_right_.entity_id.c_str() : "none");
  ESP_LOGCONFIG(TAG, "  Cards: %u", static_cast<unsigned>(this->cards_.size()));
  ESP_LOGCONFIG(TAG, "  TFT: %s %s", this->tft_version_.empty() ? "unknown" : this->tft_version_.c_str(),
                this->tft_model_.empty() ? "unknown" : this->tft_model_.c_str());
}

void NSPanelLovelace::apply_display_settings_() {
  this->send_display_command("timeout~" + std::to_string(this->sleep_timeout_));
  this->send_display_command("dimmode~" + std::to_string(this->screensaver_brightness_) + "~" +
                             std::to_string(this->active_brightness_));
}

std::string NSPanelLovelace::translate_datetime_(std::string value) const {
  static const std::array<std::pair<const char *, const char *>, 23> MONTHS{{
      {"January", "month_january"}, {"Jan", "month_jan"},           {"February", "month_february"},
      {"Feb", "month_feb"},       {"March", "month_march"},       {"Mar", "month_mar"},
      {"April", "month_april"},   {"Apr", "month_apr"},           {"May", "month_may"},
      {"June", "month_june"},     {"Jun", "month_jun"},           {"July", "month_july"},
      {"Jul", "month_jul"},       {"August", "month_august"},     {"Aug", "month_aug"},
      {"September", "month_september"}, {"Sep", "month_sep"},     {"October", "month_october"},
      {"Oct", "month_oct"},       {"November", "month_november"}, {"Nov", "month_nov"},
      {"December", "month_december"}, {"Dec", "month_dec"},
  }};
  static const std::array<std::pair<const char *, const char *>, 14> DAYS{{
      {"Sunday", "dow_sunday"},    {"Sun", "dow_sun"},       {"Monday", "dow_monday"},
      {"Mon", "dow_mon"},          {"Tuesday", "dow_tuesday"}, {"Tue", "dow_tue"},
      {"Wednesday", "dow_wednesday"}, {"Wed", "dow_wed"},    {"Thursday", "dow_thursday"},
      {"Thu", "dow_thu"},          {"Friday", "dow_friday"}, {"Fri", "dow_fri"},
      {"Saturday", "dow_saturday"}, {"Sat", "dow_sat"},
  }};

  for (const auto &translation : MONTHS) {
    replace_first(value, translation.first, this->get_translation_(translation.second));
  }
  for (const auto &translation : DAYS) {
    replace_first(value, translation.first, this->get_translation_(translation.second));
  }
  return value;
}

std::string NSPanelLovelace::get_translation_(const std::string &key) const {
  const auto translation = this->translations_.find(key);
  if (translation != this->translations_.end()) {
    return translation->second;
  }
  return key;
}

std::string NSPanelLovelace::protocol_escape_(const std::string &value) {
  std::string escaped = value;
  std::replace(escaped.begin(), escaped.end(), '~', '-');
  return escaped;
}

std::vector<std::string> NSPanelLovelace::split_(const std::string &value, char separator) {
  std::vector<std::string> parts;
  size_t start = 0;
  while (start <= value.size()) {
    const auto end = value.find(separator, start);
    if (end == std::string::npos) {
      parts.push_back(value.substr(start));
      break;
    }
    parts.push_back(value.substr(start, end - start));
    start = end + 1;
  }
  return parts;
}

}  // namespace nspanel_lovelace
}  // namespace esphome
