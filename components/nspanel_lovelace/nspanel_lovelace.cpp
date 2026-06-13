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
  ESP_LOGCONFIG(TAG, "  Language: %s", this->language_.c_str());
  ESP_LOGCONFIG(TAG, "  Screensaver: %s", YESNO(this->screensaver_enabled_));
  ESP_LOGCONFIG(TAG, "  Time Source: %s", this->time_ == nullptr ? "none" : "configured");
  ESP_LOGCONFIG(TAG, "  Screensaver Weather: %s", this->screensaver_weather_.enabled ? this->screensaver_weather_.entity_id.c_str() : "none");
  ESP_LOGCONFIG(TAG, "  Screensaver Entities: %zu", this->screensaver_entities_.size());
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

  this->send_display_command("time~" + this->translate_datetime_(now.strftime(this->time_format_)));
  this->send_display_command("date~" + this->translate_datetime_(now.strftime(this->date_format_)));
}

void NSPanelLovelace::subscribe_screensaver_entities_() {
  for (const auto &entity : this->screensaver_entities_) {
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_entity_state_, entity.entity_id);
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
  if (!this->screensaver_enabled_ ||
      (!this->screensaver_weather_.enabled && this->screensaver_forecast_.items.empty() &&
       this->screensaver_entities_.empty())) {
    return;
  }

  std::string command{"weatherUpdate"};
  if (this->screensaver_weather_.enabled) {
    const auto icon = this->weather_icon_for_condition_(this->screensaver_weather_.state, this->screensaver_weather_.color);
    this->append_screensaver_item_(command, icon.icon, icon.color, "",
                                   this->screensaver_weather_.temperature + this->screensaver_weather_.temperature_unit);
  }

  for (const auto &forecast : this->screensaver_forecast_.items) {
    this->append_screensaver_item_(command, forecast.icon, forecast.color, forecast.name,
                                   forecast.value + this->screensaver_weather_.temperature_unit);
  }

  for (const auto &entity : this->screensaver_entities_) {
    this->append_screensaver_item_(command, entity.icon, entity.color, entity.name, entity.state);
  }
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

}  // namespace nspanel_lovelace
}  // namespace esphome
