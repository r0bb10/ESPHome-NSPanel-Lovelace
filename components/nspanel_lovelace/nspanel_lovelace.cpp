#include "nspanel_lovelace.h"

#include <algorithm>
#include <array>
#include <ctime>

#include "esphome/components/json/json_util.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

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

void NSPanelLovelace::set_screensaver_weather(std::string entity_id, int32_t color) {
  this->screensaver_weather_.enabled = true;
  this->screensaver_weather_.entity_id = std::move(entity_id);
  this->screensaver_weather_.color = color;
}

void NSPanelLovelace::set_screensaver_forecast(std::string entity_id, int32_t color) {
  this->screensaver_forecast_.enabled = true;
  this->screensaver_forecast_.entity_id = std::move(entity_id);
  this->screensaver_forecast_.color = color;
}

void NSPanelLovelace::set_screensaver_extra_entity(std::string entity_id, std::string icon, uint16_t color) {
  this->screensaver_extra_entity_.enabled = true;
  this->screensaver_extra_entity_.entity_id = std::move(entity_id);
  this->screensaver_extra_entity_.icon = std::move(icon);
  this->screensaver_extra_entity_.color = color;
}

void NSPanelLovelace::set_screensaver_status_icon_left(std::string entity_id, std::string icon, uint16_t color,
                                                       bool alt_font) {
  this->screensaver_status_icon_left_ = ScreensaverStatusIcon{true, std::move(entity_id), std::move(icon), color, alt_font};
}

void NSPanelLovelace::set_screensaver_status_icon_right(std::string entity_id, std::string icon, uint16_t color,
                                                        bool alt_font) {
  this->screensaver_status_icon_right_ = ScreensaverStatusIcon{true, std::move(entity_id), std::move(icon), color, alt_font};
}

void NSPanelLovelace::add_card_entities(std::string type, std::string title) {
  this->cards_.push_back(CardPage{std::move(type), std::move(title), "", {}});
}

void NSPanelLovelace::add_card_qr(std::string title, std::string qr_text) {
  this->cards_.push_back(CardPage{"cardQR", std::move(title), std::move(qr_text), {}});
}

void NSPanelLovelace::add_card_entity(std::string entity_id, std::string name, std::string icon, uint16_t color) {
  if (this->cards_.empty()) {
    return;
  }
  if (name.empty()) {
    name = entity_id;
  }
  this->cards_.back().entities.push_back(CardEntity{std::move(entity_id), std::move(name), std::move(icon), color, ""});
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

  this->send_display_command("time~" + this->translate_datetime_(now.strftime(this->time_format_)));
  this->send_display_command("date~" + this->translate_datetime_(now.strftime(this->date_format_)));
}

void NSPanelLovelace::subscribe_screensaver_extra_entity_() {
  if (this->screensaver_extra_entity_.enabled) {
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_extra_entity_state_,
                                        this->screensaver_extra_entity_.entity_id);
  }
}

void NSPanelLovelace::subscribe_screensaver_status_icons_() {
  if (this->screensaver_status_icon_left_.enabled) {
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_status_icon_state_,
                                        this->screensaver_status_icon_left_.entity_id);
  }
  if (this->screensaver_status_icon_right_.enabled) {
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_status_icon_state_,
                                        this->screensaver_status_icon_right_.entity_id);
  }
}

void NSPanelLovelace::subscribe_screensaver_weather_() {
  if (this->screensaver_weather_.enabled) {
    const auto &entity_id = this->screensaver_weather_.entity_id;
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_weather_state_, entity_id);
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_weather_temperature_, entity_id, "temperature");
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_weather_temperature_unit_, entity_id,
                                        "temperature_unit");
  }

  if (this->screensaver_forecast_.enabled) {
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_forecast_, this->screensaver_forecast_.entity_id,
                                        "forecast");
  }
}

void NSPanelLovelace::subscribe_card_entities_() {
  for (const auto &card : this->cards_) {
    for (const auto &entity : card.entities) {
      this->subscribe_homeassistant_state(&NSPanelLovelace::on_card_entity_state_, entity.entity_id);
    }
  }
}

void NSPanelLovelace::on_screensaver_weather_state_(const std::string &entity_id, StringRef state) {
  if (!this->screensaver_weather_.enabled || this->screensaver_weather_.entity_id != entity_id) {
    return;
  }

  this->screensaver_weather_.state = state.str();
  this->render_screensaver_entities_();
}

void NSPanelLovelace::on_screensaver_weather_temperature_(const std::string &entity_id, StringRef temperature) {
  if (!this->screensaver_weather_.enabled || this->screensaver_weather_.entity_id != entity_id) {
    return;
  }

  this->screensaver_weather_.temperature = temperature.str();
  this->render_screensaver_entities_();
}

void NSPanelLovelace::on_screensaver_weather_temperature_unit_(const std::string &entity_id, StringRef temperature_unit) {
  if (!this->screensaver_weather_.enabled || this->screensaver_weather_.entity_id != entity_id) {
    return;
  }

  this->screensaver_weather_.temperature_unit = temperature_unit.str();
  this->render_screensaver_entities_();
}

void NSPanelLovelace::on_screensaver_forecast_(const std::string &entity_id, StringRef forecast_json) {
  if (!this->screensaver_forecast_.enabled || this->screensaver_forecast_.entity_id != entity_id) {
    return;
  }

  ArduinoJson::JsonDocument doc;
  const auto error = ArduinoJson::deserializeJson(doc, forecast_json.c_str());
  if (error || doc.overflowed()) {
    ESP_LOGW(TAG, "Weather forecast JSON unparsable: %s", error ? error.c_str() : "document overflow");
    return;
  }

  ArduinoJson::JsonArray forecast;
  if (doc.is<ArduinoJson::JsonArray>()) {
    forecast = doc.as<ArduinoJson::JsonArray>();
  } else if (doc.is<ArduinoJson::JsonObject>() && doc["forecast"].is<ArduinoJson::JsonArray>()) {
    forecast = doc["forecast"].as<ArduinoJson::JsonArray>();
  } else {
    ESP_LOGW(TAG, "Weather forecast JSON is not an array or object with forecast key");
    return;
  }

  bool hourly = false;
  if (forecast.size() > 1) {
    tm first{};
    tm second{};
    if (parse_iso8601_(forecast[0]["datetime"].as<const char *>(), first) &&
        parse_iso8601_(forecast[1]["datetime"].as<const char *>(), second)) {
      hourly = first.tm_hour != second.tm_hour;
    }
  }

  this->screensaver_forecast_.items.clear();
  this->screensaver_forecast_.items.reserve(4);
  for (const auto item : forecast) {
    if (this->screensaver_forecast_.items.size() >= 4) {
      break;
    }

    tm forecast_time{};
    const char *datetime = item["datetime"].as<const char *>();
    if (!parse_iso8601_(datetime, forecast_time)) {
      ESP_LOGW(TAG, "Weather forecast datetime unparsable: %s", datetime == nullptr ? "" : datetime);
      continue;
    }

    char temperature[16]{};
    snprintf(temperature, sizeof(temperature), "%.1f", item["temperature"].as<float>());
    const auto icon = this->weather_icon_for_condition_(item["condition"].as<std::string>(), this->screensaver_forecast_.color);
    this->screensaver_forecast_.items.push_back(ScreensaverForecastItem{icon.icon,
                                                                        icon.color,
                                                                        this->format_forecast_time_(forecast_time, hourly),
                                                                        temperature});
  }

  this->render_screensaver_entities_();
}

void NSPanelLovelace::on_screensaver_extra_entity_state_(const std::string &entity_id, StringRef state) {
  if (!this->screensaver_extra_entity_.enabled || this->screensaver_extra_entity_.entity_id != entity_id) {
    return;
  }

  this->screensaver_extra_entity_.state = state.str();
  this->render_screensaver_entities_();
}

void NSPanelLovelace::on_screensaver_status_icon_state_(const std::string &entity_id, StringRef state) {
  if ((this->screensaver_status_icon_left_.enabled && this->screensaver_status_icon_left_.entity_id == entity_id) ||
      (this->screensaver_status_icon_right_.enabled && this->screensaver_status_icon_right_.entity_id == entity_id)) {
    this->render_screensaver_status_icons_();
  }
}

void NSPanelLovelace::on_card_entity_state_(const std::string &entity_id, StringRef state) {
  for (auto &card : this->cards_) {
    for (auto &entity : card.entities) {
      if (entity.entity_id == entity_id) {
        entity.state = state.str();
      }
    }
  }
  this->render_current_card_();
}

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

void NSPanelLovelace::show_card_(size_t index) {
  if (index >= this->cards_.size()) {
    return;
  }
  this->current_card_ = index;
  this->card_visible_ = true;
  this->send_display_command("pageType~" + this->cards_[index].type);
  this->render_current_card_();
}

void NSPanelLovelace::show_screensaver_from_event_() {
  this->card_visible_ = false;
  this->show_screensaver_();
  this->update_datetime_();
  this->render_screensaver_entities_();
  this->render_screensaver_status_icons_();
}

void NSPanelLovelace::render_current_card_() {
  if (!this->card_visible_ || this->current_card_ >= this->cards_.size()) {
    return;
  }

  const auto &card = this->cards_[this->current_card_];
  std::string command{"entityUpd~"};
  command.append(protocol_escape_(card.title)).append("~");
  this->render_card_navigation_(command);
  if (card.type == "cardQR") {
    command.append("~").append(protocol_escape_(card.qr_text));
  }
  for (const auto &entity : card.entities) {
    this->append_card_entity_(command, entity);
  }
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::render_card_navigation_(std::string &command) const {
  if (this->cards_.size() <= 1) {
    command.append("delete~~~~~~delete~~~~~");
    return;
  }

  const auto prev = this->current_card_ == 0 ? this->cards_.size() - 1 : this->current_card_ - 1;
  const auto next = (this->current_card_ + 1) % this->cards_.size();
  command.append("button~navigate.uuid.")
      .append(std::to_string(prev))
      .append("~mdi:arrow-left-bold~65535~~")
      .append("~button~navigate.uuid.")
      .append(std::to_string(next))
      .append("~mdi:arrow-right-bold~65535~~");
}

void NSPanelLovelace::append_card_entity_(std::string &command, const CardEntity &entity) const {
  command.append("~")
      .append(entity_render_type_(entity.entity_id))
      .append("~")
      .append(protocol_escape_(entity.entity_id))
      .append("~")
      .append(protocol_escape_(entity.icon))
      .append("~")
      .append(std::to_string(entity.color))
      .append("~")
      .append(protocol_escape_(entity.name))
      .append("~")
      .append(protocol_escape_(entity_value_(entity)));
}

void NSPanelLovelace::render_screensaver_entities_() {
  if (!this->screensaver_enabled_ ||
      (!this->screensaver_weather_.enabled && this->screensaver_forecast_.items.empty() &&
       !this->screensaver_extra_entity_.enabled)) {
    return;
  }

  std::string command{"weatherUpdate"};
  if (this->screensaver_weather_.enabled) {
    const auto icon = this->weather_icon_for_condition_(this->screensaver_weather_.state, this->screensaver_weather_.color);
    this->append_screensaver_item_(command, icon.icon, icon.color, "",
                                   this->screensaver_weather_.temperature + this->screensaver_weather_.temperature_unit);
  }

  size_t forecast_count = 0;
  for (const auto &forecast : this->screensaver_forecast_.items) {
    this->append_screensaver_item_(command, forecast.icon, forecast.color, forecast.name,
                                   forecast.value + this->screensaver_weather_.temperature_unit);
    ++forecast_count;
  }

  if (this->screensaver_extra_entity_.enabled) {
    while (forecast_count < 4) {
      this->append_screensaver_item_(command, "", 63878, "", "");
      ++forecast_count;
    }
    this->append_screensaver_item_(command, this->screensaver_extra_entity_.icon, this->screensaver_extra_entity_.color,
                                   "", this->screensaver_extra_entity_.state);
  }
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::render_screensaver_status_icons_() {
  if (!this->screensaver_enabled_ ||
      (!this->screensaver_status_icon_left_.enabled && !this->screensaver_status_icon_right_.enabled)) {
    return;
  }

  std::string command{"statusUpdate"};
  append_status_icon_(command, this->screensaver_status_icon_left_);
  append_status_icon_(command, this->screensaver_status_icon_right_);
  command.append("~")
      .append(this->screensaver_status_icon_left_.enabled && this->screensaver_status_icon_left_.alt_font ? "1" : "")
      .append("~")
      .append(this->screensaver_status_icon_right_.enabled && this->screensaver_status_icon_right_.alt_font ? "1" : "");
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::append_screensaver_item_(std::string &command, const std::string &icon, uint16_t color,
                                               const std::string &name, const std::string &value) {
  command.append("~~~")
      .append(protocol_escape_(icon))
      .append("~")
      .append(std::to_string(color))
      .append("~")
      .append(protocol_escape_(name))
      .append("~")
      .append(protocol_escape_(value));
}

void NSPanelLovelace::append_status_icon_(std::string &command, const ScreensaverStatusIcon &icon) {
  command.append("~");
  if (icon.enabled) {
    command.append(protocol_escape_(icon.icon)).append("~").append(std::to_string(icon.color));
  } else {
    command.append("~");
  }
}

WeatherIcon NSPanelLovelace::weather_icon_for_condition_(const std::string &condition, int32_t color_override) const {
  const auto color = [color_override](uint16_t default_color) -> uint16_t {
    return color_override >= 0 ? static_cast<uint16_t>(color_override) : default_color;
  };
  if (condition == "sunny") {
    return WeatherIcon{"\xEE\x96\x98", color(65504)};
  }
  if (condition == "windy") {
    return WeatherIcon{"\xEE\x96\x9C", color(38066)};
  }
  if (condition == "windy-variant") {
    return WeatherIcon{"\xEE\x96\x9D", color(64495)};
  }
  if (condition == "cloudy") {
    return WeatherIcon{"\xEE\x96\x8F", color(31728)};
  }
  if (condition == "partlycloudy") {
    return WeatherIcon{"\xEE\x96\x94", color(38066)};
  }
  if (condition == "clear-night") {
    return WeatherIcon{"\xEE\x96\x93", color(38060)};
  }
  if (condition == "exceptional") {
    return WeatherIcon{"\xEE\x97\x95", color(63878)};
  }
  if (condition == "rainy") {
    return WeatherIcon{"\xEE\x96\x96", color(25375)};
  }
  if (condition == "pouring") {
    return WeatherIcon{"\xEE\x96\x95", color(12703)};
  }
  if (condition == "snowy") {
    return WeatherIcon{"\xEE\x96\x97", color(65535)};
  }
  if (condition == "snowy-rainy") {
    return WeatherIcon{"\xEE\xBC\xB4", color(38079)};
  }
  if (condition == "fog") {
    return WeatherIcon{"\xEE\x96\x90", color(38066)};
  }
  if (condition == "hail") {
    return WeatherIcon{"\xEE\x96\x91", color(65535)};
  }
  if (condition == "lightning") {
    return WeatherIcon{"\xEE\x96\x92", color(65120)};
  }
  if (condition == "lightning-rainy") {
    return WeatherIcon{"\xEE\x99\xBD", color(50400)};
  }
  return WeatherIcon{"\xEE\x97\x95", color(63878)};
}

bool NSPanelLovelace::parse_iso8601_(const char *value, tm &time) {
  if (value == nullptr) {
    return false;
  }

  char *end = strptime(value, "%Y-%m-%dT%H:%M:%S", &time);
  if (end != nullptr) {
    mktime(&time);
    return true;
  }
  end = strptime(value, "%Y-%m-%d", &time);
  if (end == nullptr) {
    return false;
  }
  mktime(&time);
  return true;
}

std::string NSPanelLovelace::format_forecast_time_(const tm &time, bool hourly) const {
  char value[32]{};
  if (hourly) {
    strftime(value, sizeof(value), this->time_format_.c_str(), &time);
    return this->translate_datetime_(value);
  }

  switch (time.tm_wday) {
    case 0:
      return this->get_translation_("dow_sun");
    case 1:
      return this->get_translation_("dow_mon");
    case 2:
      return this->get_translation_("dow_tue");
    case 3:
      return this->get_translation_("dow_wed");
    case 4:
      return this->get_translation_("dow_thu");
    case 5:
      return this->get_translation_("dow_fri");
    case 6:
      return this->get_translation_("dow_sat");
    default:
      return "";
  }
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

std::string NSPanelLovelace::entity_domain_(const std::string &entity_id) {
  const auto pos = entity_id.find('.');
  if (pos == std::string::npos) {
    return entity_id;
  }
  return entity_id.substr(0, pos);
}

std::string NSPanelLovelace::entity_render_type_(const std::string &entity_id) {
  const auto domain = entity_domain_(entity_id);
  if (domain == "light") {
    return "light";
  }
  if (domain == "switch" || domain == "input_boolean" || domain == "automation") {
    return "switch";
  }
  if (domain == "fan") {
    return "fan";
  }
  if (domain == "button" || domain == "input_button" || domain == "scene" || domain == "script") {
    return "button";
  }
  if (domain == "number" || domain == "input_number") {
    return "number";
  }
  if (domain == "input_select" || domain == "select") {
    return "input_sel";
  }
  return "text";
}

std::string NSPanelLovelace::entity_value_(const CardEntity &entity) {
  const auto domain = entity_domain_(entity.entity_id);
  if (domain == "light" || domain == "switch" || domain == "input_boolean" || domain == "automation" ||
      domain == "fan") {
    return entity.state == "on" ? "1" : "0";
  }
  if (domain == "button" || domain == "input_button") {
    return "press";
  }
  if (domain == "scene") {
    return "activate";
  }
  if (domain == "script") {
    return "run";
  }
  return entity.state;
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
