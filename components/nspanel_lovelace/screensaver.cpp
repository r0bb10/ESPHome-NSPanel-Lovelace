#include "nspanel_lovelace.h"

#include <ctime>

#include "esphome/components/json/json_util.h"
#include "icons.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace.screensaver";

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

void NSPanelLovelace::show_screensaver_from_event_() {
  this->card_visible_ = false;
  this->show_screensaver_();
  this->update_datetime_();
  this->render_screensaver_entities_();
  this->render_screensaver_status_icons_();
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
    this->screensaver_forecast_.items.push_back(ScreensaverForecastItem{icon.icon, icon.color,
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
    const auto icon = this->screensaver_extra_entity_.icon.empty()
                          ? ""
                          : icons::resolve_icon(this->screensaver_extra_entity_.icon);
    this->append_screensaver_item_(command, icon, this->screensaver_extra_entity_.color, "",
                                   this->screensaver_extra_entity_.state);
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
    command.append(protocol_escape_(icons::resolve_icon(icon.icon))).append("~").append(std::to_string(icon.color));
  } else {
    command.append("~");
  }
}

WeatherIcon NSPanelLovelace::weather_icon_for_condition_(const std::string &condition, int32_t color_override) const {
  const auto color = [color_override](uint16_t default_color) -> uint16_t {
    return color_override >= 0 ? static_cast<uint16_t>(color_override) : default_color;
  };
  if (condition == "sunny") return WeatherIcon{icons::WEATHER_SUNNY, color(65504)};
  if (condition == "windy") return WeatherIcon{icons::WEATHER_WINDY, color(38066)};
  if (condition == "windy-variant") return WeatherIcon{icons::WEATHER_WINDY_VARIANT, color(64495)};
  if (condition == "cloudy") return WeatherIcon{icons::WEATHER_CLOUDY, color(31728)};
  if (condition == "partlycloudy") return WeatherIcon{icons::WEATHER_PARTLY_CLOUDY, color(38066)};
  if (condition == "clear-night") return WeatherIcon{icons::WEATHER_NIGHT, color(38060)};
  if (condition == "exceptional") return WeatherIcon{icons::WEATHER_SUNNY, color(63878)};
  if (condition == "rainy") return WeatherIcon{icons::WEATHER_RAINY, color(25375)};
  if (condition == "pouring") return WeatherIcon{icons::WEATHER_POURING, color(12703)};
  if (condition == "snowy") return WeatherIcon{icons::WEATHER_SNOWY, color(65535)};
  if (condition == "snowy-rainy") return WeatherIcon{icons::WEATHER_SNOWY_RAINY, color(38079)};
  if (condition == "fog") return WeatherIcon{icons::WEATHER_FOG, color(38066)};
  if (condition == "hail") return WeatherIcon{icons::WEATHER_HAIL, color(65535)};
  if (condition == "lightning") return WeatherIcon{icons::WEATHER_LIGHTNING, color(65120)};
  if (condition == "lightning-rainy") return WeatherIcon{icons::WEATHER_LIGHTNING_RAINY, color(50400)};
  return WeatherIcon{icons::WEATHER_SUNNY, color(63878)};
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
    case 0: return this->get_translation_("dow_sun");
    case 1: return this->get_translation_("dow_mon");
    case 2: return this->get_translation_("dow_tue");
    case 3: return this->get_translation_("dow_wed");
    case 4: return this->get_translation_("dow_thu");
    case 5: return this->get_translation_("dow_fri");
    case 6: return this->get_translation_("dow_sat");
    default: return "";
  }
}

}  // namespace nspanel_lovelace
}  // namespace esphome
