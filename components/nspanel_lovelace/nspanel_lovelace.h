#pragma once

#include <cstdint>
#include <ctime>
#include <map>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "nextion_transport.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace nspanel_lovelace {

struct ScreensaverExtraEntity {
  bool enabled{false};
  std::string entity_id;
  std::string icon;
  uint16_t color{0xFFFF};
  std::string state;
  std::map<std::string, std::string> attributes;
};

struct ScreensaverWeather {
  bool enabled{false};
  std::string entity_id;
  int32_t color{-1};
  std::string state;
  std::string temperature;
  std::string temperature_unit;
};

struct ScreensaverForecastItem {
  std::string icon;
  uint16_t color{0xF986};
  std::string name;
  std::string value;

  bool operator==(const ScreensaverForecastItem &other) const {
    return this->icon == other.icon && this->color == other.color && this->name == other.name && this->value == other.value;
  }
  bool operator!=(const ScreensaverForecastItem &other) const { return !(*this == other); }
};

struct ScreensaverForecast {
  bool enabled{false};
  std::string entity_id;
  int32_t color{-1};
  std::vector<ScreensaverForecastItem> items;
};

struct WeatherIcon {
  std::string icon;
  uint16_t color;
};

struct ScreensaverStatusIcon {
  bool enabled{false};
  std::string entity_id;
  std::string icon;
  uint16_t color{0xFFFF};
  bool alt_font{false};
};

struct CardEntity {
  std::string entity_id;
  std::string name;
  std::string icon;
  uint16_t color{0x4393};
  std::string state;
  std::map<std::string, std::string> attributes;
  std::string value;  // static text value used by cardQR rows
};

struct CardPage {
  std::string type;
  std::string title;
  std::string qr_text;
  std::string ssid;
  std::string password;
  std::string auth{"WPA"};
  std::vector<CardEntity> entities;
};

class NSPanelLovelace : public Component, public uart::UARTDevice, public api::CustomAPIDevice {
 public:
  // --- Lifecycle ---
  void setup() override;
  void loop() override;
  void dump_config() override;

  // --- Configuration setters ---
  void set_model(const std::string &model) { this->model_ = model; }
  void set_sleep_timeout(uint16_t sleep_timeout) { this->sleep_timeout_ = sleep_timeout; }
  void set_active_brightness(uint8_t active_brightness) { this->active_brightness_ = active_brightness; }
  void set_screensaver_brightness(uint8_t screensaver_brightness) { this->screensaver_brightness_ = screensaver_brightness; }
  void set_screensaver_enabled(bool screensaver_enabled) { this->screensaver_enabled_ = screensaver_enabled; }
  void set_time(time::RealTimeClock *time) { this->time_ = time; }
  void set_time_format(const std::string &time_format) { this->time_format_ = time_format; }
  void set_date_format(const std::string &date_format) { this->date_format_ = date_format; }
  void set_language(const std::string &language) { this->language_ = language; }
  void set_translation(std::string key, std::string value) { this->translations_[std::move(key)] = std::move(value); }

  // --- Runtime API (usable in lambdas) ---
  void set_display_active_dim(uint8_t brightness);
  void set_display_inactive_dim(uint8_t brightness);

  // --- Screensaver builders ---
  void set_screensaver_weather(std::string entity_id, int32_t color);
  void set_screensaver_forecast(std::string entity_id, int32_t color);
  void set_screensaver_extra_entity(std::string entity_id, std::string icon, uint16_t color);
  void set_screensaver_status_icon_left(std::string entity_id, std::string icon, uint16_t color, bool alt_font);
  void set_screensaver_status_icon_right(std::string entity_id, std::string icon, uint16_t color, bool alt_font);

  // --- Card builders ---
  void add_card_entities(std::string type, std::string title);
  void add_card_qr(std::string title, std::string qr_text, std::string ssid = "", std::string password = "",
                   std::string auth = "WPA");
  void add_card_thermo(std::string title, std::string entity_id);
  void add_card_alarm(std::string title, std::string entity_id);
  void add_card_media(std::string title, std::string entity_id);
  void add_card_entity(std::string entity_id, std::string name, std::string icon, uint16_t color,
                       std::string value = "");

  // --- Command ---
  void send_display_command(std::string command) { this->command_queue_.push(std::move(command)); }

#ifdef USE_NSPANEL_TFT_UPLOAD
  // --- TFT upload ---
  bool upload_tft(const std::string &url);
  void register_tft_upload_service();
  void upload_tft_service_(std::string url);
#endif

 protected:
  // --- Setup / lifecycle ---
  void apply_display_settings_();
  void show_screensaver_();
  void show_screensaver_from_event_();
  void update_datetime_();

  // --- Subscriptions ---
  void subscribe_homeassistant_state_attr_(const std::string &entity_id, const std::string &attr_name);
  void subscribe_screensaver_weather_();
  void subscribe_screensaver_extra_entity_();
  void subscribe_screensaver_status_icons_();
  void subscribe_card_entities_();

  // --- Inbound event handling (TFT -> us) ---
  void process_display_messages_();
  void process_display_message_(const std::string &message);
  void handle_startup_event_(const std::vector<std::string> &parts);
  void handle_sleep_reached_event_();
  void handle_button_press_event_(const std::vector<std::string> &parts);
  void handle_page_open_detail_event_(const std::vector<std::string> &parts);
  void handle_navigation_button_(const std::string &internal_id);
  void handle_entity_action_(const std::string &entity_id, const std::string &button_type,
                             const std::string &value);
  bool handle_detail_action_(const std::string &entity_id, const std::string &button_type,
                             const std::string &value);

  // --- HA service calls (us -> HA) ---
  void call_ha_service_(const std::string &service, const std::string &entity_id);
  void call_ha_service_(const std::string &service, const std::map<std::string, std::string> &data);
  void call_ha_service_(const std::string &domain, const std::string &service,
                        const std::map<std::string, std::string> &data);

  // --- HA state callbacks (HA -> us) ---
  void on_screensaver_weather_state_(const std::string &entity_id, StringRef state);
  void on_screensaver_weather_temperature_(const std::string &entity_id, StringRef temperature);
  void on_screensaver_weather_temperature_unit_(const std::string &entity_id, StringRef temperature_unit);
  void on_screensaver_forecast_(const std::string &entity_id, StringRef forecast_json);
  void on_screensaver_extra_entity_state_(const std::string &entity_id, StringRef state);
  void on_screensaver_extra_entity_attr_(const std::string &entity_id, const std::string &attr, StringRef value);
  void on_screensaver_status_icon_state_(const std::string &entity_id, StringRef state);
  void on_card_entity_state_(const std::string &entity_id, StringRef state);
  void on_card_entity_attr_(const std::string &entity_id, const std::string &attr, StringRef value);

  // --- Card rendering (us -> TFT) ---
  CardEntity *find_card_entity_(const std::string &entity_id);
  void show_card_(size_t index);
  void render_current_card_();
  void render_card_navigation_(std::string &command) const;
  void append_card_entity_(std::string &command, const CardEntity &entity) const;
  void append_qr_row_(std::string &command, const CardEntity &entity) const;
  std::string build_qr_text_(const CardPage &card) const;
  void render_card_thermo_(const CardPage &card);
  void render_card_alarm_(const CardPage &card);
  void render_card_media_(const CardPage &card);
  void show_popup_(const std::string &page_type, const std::string &entity_id);
  void close_popup_();
  void render_popup_();
  void render_light_detail_(const CardEntity &entity);
  void render_cover_detail_(const CardEntity &entity);
  void render_fan_detail_(const CardEntity &entity);
  void render_select_detail_(const CardEntity &entity);
  void render_climate_detail_(const CardEntity &entity);

  // --- Screensaver rendering (us -> TFT) ---
  void schedule_screensaver_entities_render_();
  void render_screensaver_content_();
  void render_screensaver_entities_();
  void render_screensaver_status_icons_();
  void append_screensaver_item_(std::string &command, const std::string &icon, uint16_t color,
                                const std::string &name, const std::string &value);
  static void append_status_icon_(std::string &command, const ScreensaverStatusIcon &icon);

  // --- Utility helpers ---
  static std::string entity_domain_(const std::string &entity_id);
  static std::string entity_render_type_(const std::string &entity_id);
  std::string entity_value_(const CardEntity &entity) const;
  static std::vector<std::string> split_(const std::string &value, char separator);
  static std::vector<std::string> split_list_attr_(const std::string &value);
  static std::string join_list_(const std::vector<std::string> &values);
  template<typename T>
  static bool parse_int_(const std::string &value, T &out) {
    char *end{nullptr};
    const auto result = strtol(value.c_str(), &end, 10);
    if (end == value.c_str() || *end != '\0') {
      return false;
    }
    out = static_cast<T>(result);
    return true;
  }
  template<typename T>
  static bool parse_float_(const std::string &value, T &out) {
    char *end{nullptr};
    const auto result = strtof(value.c_str(), &end);
    if (end == value.c_str() || *end != '\0') {
      return false;
    }
    out = static_cast<T>(result);
    return true;
  }
  static std::string join_rgb_(const std::vector<uint8_t> &rgb);
  WeatherIcon weather_icon_for_condition_(const std::string &condition, int32_t color_override) const;
  static bool parse_iso8601_(const char *value, tm &time);
  std::string format_forecast_time_(const tm &time, bool hourly) const;
  std::string translate_datetime_(std::string value) const;
  std::string get_translation_(const std::string &key) const;
  static std::string protocol_escape_(const std::string &value);

#ifdef USE_NSPANEL_TFT_UPLOAD
  // --- TFT upload internals ---
  uint16_t recv_ret_string_(std::string &response, uint32_t timeout);
  bool upload_end_(bool successful);
#endif

  // --- Member variables ---
  std::string model_{"eu"};
  uint16_t sleep_timeout_{20};
  uint8_t active_brightness_{100};
  uint8_t screensaver_brightness_{20};
  bool screensaver_enabled_{false};
  time::RealTimeClock *time_{nullptr};
  std::string language_{"en"};
  std::map<std::string, std::string> translations_;
  std::string time_format_{"%H:%M"};
  std::string date_format_{"%A, %d. %B %Y"};
  uint32_t last_datetime_update_{0};

  ScreensaverWeather screensaver_weather_;
  ScreensaverForecast screensaver_forecast_;
  ScreensaverExtraEntity screensaver_extra_entity_;
  ScreensaverStatusIcon screensaver_status_icon_left_;
  ScreensaverStatusIcon screensaver_status_icon_right_;

  std::vector<CardPage> cards_;
  size_t current_card_{0};
  bool card_visible_{false};
  std::string popup_entity_id_;
  std::string popup_page_type_;

  bool display_started_{false};
  std::string tft_version_;
  std::string tft_model_;
  NextionTransport transport_;
  std::queue<std::string> command_queue_;

#ifdef USE_NSPANEL_TFT_UPLOAD
  uint32_t original_baud_rate_{0};
  bool is_updating_{false};
  uint32_t content_length_{0};
  size_t tft_size_{0};
  bool upload_first_chunk_sent_{false};
#endif
};

}  // namespace nspanel_lovelace
}  // namespace esphome
