#include "nspanel_lovelace.h"

#include <ctime>

#include "esphome/components/json/json_util.h"
#include "icons.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace.screensaver";

// --- Public: Screensaver builders ---

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

// --- Setup / lifecycle ---

void NSPanelLovelace::show_screensaver_() {
  if (!this->screensaver_enabled_) {
    return;
  }

  this->send_display_command("pageType~screensaver");
}

void NSPanelLovelace::show_screensaver_from_event_() {
  this->card_visible_ = false;
  this->show_screensaver_();
  this->render_screensaver_content_();
  this->update_datetime_();
}

void NSPanelLovelace::update_datetime_() {
  const uint32_t now_millis = millis();
  this->last_datetime_update_ = now_millis;
  if (this->time_ == nullptr) {
    return;
  }

  auto now = this->time_->now();
  if (!now.is_valid()) {
    if (this->last_valid_time_epoch_ == 0) {
      return;
    }
    now = ESPTime::from_epoch_local(this->last_valid_time_epoch_ + ((now_millis - this->last_valid_time_millis_) / 1000));
    if (!now.is_valid()) {
      return;
    }
  } else {
    this->last_valid_time_epoch_ = now.timestamp;
    this->last_valid_time_millis_ = now_millis;
  }

  this->send_display_command("time~" + this->translate_datetime_(now.strftime(this->time_format_)));
  this->send_display_command("date~" + this->translate_datetime_(now.strftime(this->date_format_)));
}

// --- Subscriptions ---

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

void NSPanelLovelace::subscribe_screensaver_extra_entity_() {
  if (this->screensaver_extra_entity_.enabled) {
    const auto &entity_id = this->screensaver_extra_entity_.entity_id;
    this->subscribe_homeassistant_state(&NSPanelLovelace::on_screensaver_extra_entity_state_, entity_id);

    auto subscribe_attr = [this, &entity_id](const std::string &attr) {
      auto f = std::bind(&NSPanelLovelace::on_screensaver_extra_entity_attr_, this, entity_id, attr,
                         std::placeholders::_1);
      api::global_api_server->subscribe_home_assistant_state(entity_id, optional<std::string>(attr), std::move(f));
    };
    subscribe_attr("device_class");
    subscribe_attr("unit_of_measurement");
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

// --- HA state callbacks (HA -> us) ---

void NSPanelLovelace::on_screensaver_weather_state_(const std::string &entity_id, StringRef state) {
  if (!this->screensaver_weather_.enabled || this->screensaver_weather_.entity_id != entity_id) {
    return;
  }

  const auto value = state.str();
  if (this->screensaver_weather_.state == value) {
    return;
  }
  this->screensaver_weather_.state = value;
  this->schedule_screensaver_entities_render_();
}

void NSPanelLovelace::on_screensaver_weather_temperature_(const std::string &entity_id, StringRef temperature) {
  if (!this->screensaver_weather_.enabled || this->screensaver_weather_.entity_id != entity_id) {
    return;
  }

  const auto value = temperature.str();
  if (this->screensaver_weather_.temperature == value) {
    return;
  }
  this->screensaver_weather_.temperature = value;
  this->schedule_screensaver_entities_render_();
}

void NSPanelLovelace::on_screensaver_weather_temperature_unit_(const std::string &entity_id, StringRef temperature_unit) {
  if (!this->screensaver_weather_.enabled || this->screensaver_weather_.entity_id != entity_id) {
    return;
  }

  const auto value = temperature_unit.str();
  if (this->screensaver_weather_.temperature_unit == value) {
    return;
  }
  this->screensaver_weather_.temperature_unit = value;
  this->schedule_screensaver_entities_render_();
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

  std::vector<ScreensaverForecastItem> items;
  items.reserve(4);
  for (const auto item : forecast) {
    if (items.size() >= 4) {
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
    items.push_back(ScreensaverForecastItem{icon.icon, icon.color, this->format_forecast_time_(forecast_time, hourly),
                                            temperature});
  }

  if (this->screensaver_forecast_.items == items) {
    return;
  }
  this->screensaver_forecast_.items = std::move(items);
  this->schedule_screensaver_entities_render_();
}

void NSPanelLovelace::on_screensaver_extra_entity_state_(const std::string &entity_id, StringRef state) {
  if (!this->screensaver_extra_entity_.enabled || this->screensaver_extra_entity_.entity_id != entity_id) {
    return;
  }

  const auto value = state.str();
  if (this->screensaver_extra_entity_.state == value) {
    return;
  }
  this->screensaver_extra_entity_.state = value;
  this->schedule_screensaver_entities_render_();
}

void NSPanelLovelace::on_screensaver_extra_entity_attr_(const std::string &entity_id, const std::string &attr,
                                                       StringRef value) {
  if (!this->screensaver_extra_entity_.enabled || this->screensaver_extra_entity_.entity_id != entity_id) {
    return;
  }
  const auto value_str = value.str();
  const auto it = this->screensaver_extra_entity_.attributes.find(attr);
  if (it != this->screensaver_extra_entity_.attributes.end() && it->second == value_str) {
    return;
  }
  this->screensaver_extra_entity_.attributes[attr] = value_str;
  this->schedule_screensaver_entities_render_();
}

void NSPanelLovelace::on_screensaver_status_icon_state_(const std::string &entity_id, StringRef state) {
  if ((this->screensaver_status_icon_left_.enabled && this->screensaver_status_icon_left_.entity_id == entity_id) ||
      (this->screensaver_status_icon_right_.enabled && this->screensaver_status_icon_right_.entity_id == entity_id)) {
    this->render_screensaver_status_icons_();
  }
}

// --- Screensaver rendering (us -> TFT) ---

void NSPanelLovelace::schedule_screensaver_entities_render_() {
  if (!this->screensaver_enabled_ || this->card_visible_) {
    return;
  }
  this->set_timeout("screensaver_entities", 50, [this]() { this->render_screensaver_entities_(); });
}

void NSPanelLovelace::render_screensaver_content_() {
  this->render_screensaver_entities_();
  this->render_screensaver_status_icons_();
}

void NSPanelLovelace::render_screensaver_entities_() {
  if (!this->screensaver_enabled_ || this->card_visible_ ||
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
      this->append_screensaver_item_(command, "", 0xF986, "", "");
      ++forecast_count;
    }
    const auto icon = this->screensaver_extra_entity_.icon.empty()
                          ? icons::icon_for_entity(this->screensaver_extra_entity_.entity_id,
                                                   this->screensaver_extra_entity_.state,
                                                   this->screensaver_extra_entity_.attributes)
                          : icons::resolve_icon(this->screensaver_extra_entity_.icon);
    const auto unit_it = this->screensaver_extra_entity_.attributes.find("unit_of_measurement");
    const auto value = this->screensaver_extra_entity_.state +
                       (unit_it == this->screensaver_extra_entity_.attributes.end() ? "" : unit_it->second);
    this->append_screensaver_item_(command, icon, this->screensaver_extra_entity_.color, "",
                                   value);
  }
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::render_screensaver_status_icons_() {
  if (!this->screensaver_enabled_ || this->card_visible_ ||
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

// --- Utility helpers ---

WeatherIcon NSPanelLovelace::weather_icon_for_condition_(const std::string &condition, int32_t color_override) const {
  const auto color = [color_override](uint16_t default_color) -> uint16_t {
    return color_override >= 0 ? static_cast<uint16_t>(color_override) : default_color;
  };
  if (condition.empty()) return WeatherIcon{"", color(0xF986)};
  if (condition == "sunny") return WeatherIcon{icons::WEATHER_SUNNY, color(0xFFE0)};
  if (condition == "windy") return WeatherIcon{icons::WEATHER_WINDY, color(0x94BA)};
  if (condition == "windy-variant") return WeatherIcon{icons::WEATHER_WINDY_VARIANT, color(0xFBEF)};
  if (condition == "cloudy") return WeatherIcon{icons::WEATHER_CLOUDY, color(0x7BF0)};
  if (condition == "partlycloudy") return WeatherIcon{icons::WEATHER_PARTLY_CLOUDY, color(0x94BA)};
  if (condition == "clear-night") return WeatherIcon{icons::WEATHER_NIGHT, color(0x94AC)};
  if (condition == "exceptional") return WeatherIcon{icons::WEATHER_SUNNY, color(0xF986)};
  if (condition == "rainy") return WeatherIcon{icons::WEATHER_RAINY, color(0x631F)};
  if (condition == "pouring") return WeatherIcon{icons::WEATHER_POURING, color(0x319F)};
  if (condition == "snowy") return WeatherIcon{icons::WEATHER_SNOWY, color(0xFFFF)};
  if (condition == "snowy-rainy") return WeatherIcon{icons::WEATHER_SNOWY_RAINY, color(0x94BF)};
  if (condition == "fog") return WeatherIcon{icons::WEATHER_FOG, color(0x94BA)};
  if (condition == "hail") return WeatherIcon{icons::WEATHER_HAIL, color(0xFFFF)};
  if (condition == "lightning") return WeatherIcon{icons::WEATHER_LIGHTNING, color(0xFE60)};
  if (condition == "lightning-rainy") return WeatherIcon{icons::WEATHER_LIGHTNING_RAINY, color(0xC4E0)};
  return WeatherIcon{"", color(0xF986)};
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
