#include "nspanel_lovelace.h"

#include <algorithm>

#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace";

void NSPanelLovelace::setup() {
  this->transport_.set_uart(this);
  this->apply_display_settings_();
  this->subscribe_screensaver_entities_();
  this->show_screensaver_();
  this->update_datetime_();
  this->render_screensaver_entities_();
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
  ESP_LOGCONFIG(TAG, "  Screensaver Entities: %zu", this->screensaver_entities_.size());
}

void NSPanelLovelace::add_screensaver_entity(std::string entity_id, std::string name, std::string icon, uint16_t color) {
  if (name.empty()) {
    name = entity_id;
  }

  this->screensaver_entities_.push_back(
      ScreensaverEntity{std::move(entity_id), std::move(name), std::move(icon), color, ""});
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

void NSPanelLovelace::subscribe_screensaver_entities_() {
  for (const auto &entity : this->screensaver_entities_) {
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_entity_state_, entity.entity_id);
  }
}

void NSPanelLovelace::on_screensaver_entity_state_(const std::string &entity_id, StringRef state) {
  const auto entity = std::find_if(this->screensaver_entities_.begin(), this->screensaver_entities_.end(),
                                  [&entity_id](const ScreensaverEntity &item) { return item.entity_id == entity_id; });
  if (entity == this->screensaver_entities_.end()) {
    return;
  }

  entity->state = state.str();
  this->render_screensaver_entities_();
}

void NSPanelLovelace::render_screensaver_entities_() {
  if (!this->screensaver_enabled_ || this->screensaver_entities_.empty()) {
    return;
  }

  std::string command{"weatherUpdate"};
  for (const auto &entity : this->screensaver_entities_) {
    command.append("~~~")
        .append(protocol_escape_(entity.icon))
        .append("~")
        .append(std::to_string(entity.color))
        .append("~")
        .append(protocol_escape_(entity.name))
        .append("~")
        .append(protocol_escape_(entity.state));
  }
  this->send_display_command(std::move(command));
}

std::string NSPanelLovelace::protocol_escape_(const std::string &value) {
  std::string escaped = value;
  std::replace(escaped.begin(), escaped.end(), '~', '-');
  return escaped;
}

}  // namespace nspanel_lovelace
}  // namespace esphome
