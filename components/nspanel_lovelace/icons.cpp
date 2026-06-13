#include "icons.h"

#include <array>
#include <map>
#include <string>

namespace esphome {
namespace nspanel_lovelace {
namespace icons {

// Common / navigation
const char *const ARROW_LEFT_BOLD = "\uE730";
const char *const ARROW_RIGHT_BOLD = "\uE733";
const char *const ARROW_UP = "\uE05C";
const char *const ARROW_DOWN = "\uE044";
const char *const ARROW_UP_BOLD = "\uE736";
const char *const ARROW_TOP_RIGHT = "\uE05B";
const char *const ARROW_BOTTOM_LEFT = "\uE041";
const char *const ARROW_EXPAND_HORIZONTAL = "\uE84D";
const char *const ARROW_COLLAPSE_HORIZONTAL = "\uE84B";
const char *const STOP = "\uE4DA";
const char *const HOME = "\uE2DB";
const char *const HOME_OUTLINE = "\uE6A0";
const char *const HOME_THERMOMETER_OUTLINE = "\uEF54";
const char *const HELP_CIRCLE_OUTLINE = "\uE624";
const char *const ALERT_CIRCLE = "\uE027";
const char *const ALERT_CIRCLE_OUTLINE = "\uE5D5";
const char *const CHECKBOX_MARKED_CIRCLE = "\uE132";
const char *const RADIOBOX_BLANK = "\uE43C";
const char *const CHECK_CIRCLE = "\uE5DF";
const char *const CIRCLE_SLICE_8 = "\uEAA4";
const char *const CHECKBOX_BLANK_CIRCLE = "\uE12E";

// Entity type defaults
const char *const LIGHTBULB = "\uE334";
const char *const LIGHT_SWITCH = "\uE97D";
const char *const FAN = "\uE20F";
const char *const SCRIPT_TEXT = "\uEBC1";
const char *const PALETTE = "\uE3D7";
const char *const GESTURE_TAP_BUTTON = "\uF2A7";
const char *const RAY_VERTEX = "\uE444";
const char *const ROBOT = "\uE6A8";
const char *const ROBOT_VACUUM = "\uE70C";
const char *const ACCOUNT = "\uE003";
const char *const TIMER_OUTLINE = "\uE51A";
const char *const TIMER = "\uF3AA";
const char *const THERMOMETER = "\uE50E";
const char *const GAUGE = "\uE299";
const char *const SIGNAL = "\uE4A1";
const char *const SMOG = "\uEA70";
const char *const FLASH = "\uE240";
const char *const CASH = "\uE113";
const char *const CALENDAR = "\uE0EC";
const char *const CALENDAR_CLOCK = "\uE0EF";
const char *const MOTION_SENSOR = "\uED90";
const char *const MOTION_SENSOR_OFF = "\uF434";

// Lock / security
const char *const LOCK = "\uE33D";
const char *const LOCK_OPEN = "\uE33E";
const char *const SHIELD = "\uE497";
const char *const SHIELD_OFF = "\uE99D";
const char *const SHIELD_HOME = "\uE689";
const char *const SHIELD_LOCK = "\uE99C";
const char *const SHIELD_AIRPLANE = "\uE6BA";
const char *const SHIELD_MOON = "\uF827";
const char *const BELL_RING = "\uE09D";

// Climate / power
const char *const POWER = "\uE424";
const char *const FIRE = "\uE237";
const char *const SNOWFLAKE = "\uE716";
const char *const WATER_PERCENT = "\uE58D";
const char *const FAN_AUTO = "\uF71C";
const char *const SUN_SNOWFLAKE_VARIANT = "\uFA78";
const char *const TEMPERATURE_CELSIUS = "\uE503";
const char *const TEMPERATURE_FAHRENHEIT = "\uE504";

// Cover
const char *const WINDOW_OPEN = "\uE5B0";
const char *const WINDOW_CLOSED = "\uE5AD";
const char *const DOOR_OPEN = "\uE81B";
const char *const DOOR_CLOSED = "\uE81A";
const char *const GARAGE_OPEN = "\uE6D9";
const char *const GARAGE = "\uE6D8";
const char *const BLINDS_OPEN = "\uF010";
const char *const BLINDS = "\uE0AB";
const char *const CURTAINS = "\uF845";
const char *const CURTAINS_CLOSED = "\uF846";
const char *const WINDOW_SHUTTER_OPEN = "\uF11D";
const char *const WINDOW_SHUTTER = "\uF11B";
const char *const GATE_OPEN = "\uF169";
const char *const GATE = "\uE298";

// Battery / plug
const char *const BATTERY = "\uE078";
const char *const BATTERY_OUTLINE = "\uE08D";
const char *const BATTERY_CHARGING = "\uE083";
const char *const POWER_PLUG = "\uE6A4";
const char *const POWER_PLUG_OFF = "\uE6A5";

// Water / moisture
const char *const WATER = "\uE58B";
const char *const WATER_OFF = "\uE58C";

// Smoke / fire safety
const char *const SMOKE_DETECTOR = "\uE391";
const char *const SMOKE_DETECTOR_ALERT = "\uF92D";
const char *const SMOKE_DETECTOR_VARIANT = "\uF80A";
const char *const SMOKE_DETECTOR_VARIANT_ALERT = "\uF92F";

// Media
const char *const SPEAKER_OFF = "\uE4C3";
const char *const MUSIC = "\uE759";
const char *const MOVIE = "\uE380";
const char *const VIDEO = "\uE566";
const char *const PLAY = "\uE409";
const char *const PAUSE = "\uE3E3";
const char *const MUSIC_NOTE = "\uE386";
const char *const MUSIC_NOTE_OFF = "\uE389";
const char *const SHUFFLE = "\uE49C";
const char *const SHUFFLE_DISABLE = "\uE49D";

// Weather
const char *const WEATHER_SUNNY = "\uE598";
const char *const WEATHER_NIGHT = "\uE593";
const char *const WEATHER_CLOUDY = "\uE58F";
const char *const WEATHER_PARTLY_CLOUDY = "\uE594";
const char *const WEATHER_PARTLY_SNOWY_RAINY = "\uEF34";
const char *const WEATHER_FOG = "\uE590";
const char *const WEATHER_HAIL = "\uE591";
const char *const WEATHER_LIGHTNING = "\uE592";
const char *const WEATHER_LIGHTNING_RAINY = "\uE67D";
const char *const WEATHER_POURING = "\uE595";
const char *const WEATHER_RAINY = "\uE596";
const char *const WEATHER_SNOWY = "\uE597";
const char *const WEATHER_SNOWY_RAINY = "\uE67E";
const char *const WEATHER_WINDY = "\uE59C";
const char *const WEATHER_WINDY_VARIANT = "\uE59D";
const char *const WEATHER_SUNSET_UP = "\uE59B";
const char *const WEATHER_SUNSET_DOWN = "\uE59A";

namespace {

static std::string domain_from_entity_id(const std::string &entity_id) {
  auto pos = entity_id.find('.');
  if (pos == std::string::npos) return "";
  return entity_id.substr(0, pos);
}

static const char *safe_attr(const std::map<std::string, std::string> &attributes, const char *key) {
  auto it = attributes.find(key);
  if (it != attributes.end()) return it->second.c_str();
  return "";
}

static const std::map<std::string, const char *> NAME_TO_ICON = {
    {"arrow-left-bold", ARROW_LEFT_BOLD},
    {"arrow-right-bold", ARROW_RIGHT_BOLD},
    {"arrow-up", ARROW_UP},
    {"arrow-down", ARROW_DOWN},
    {"arrow-up-bold", ARROW_UP_BOLD},
    {"arrow-top-right", ARROW_TOP_RIGHT},
    {"arrow-bottom-left", ARROW_BOTTOM_LEFT},
    {"arrow-expand-horizontal", ARROW_EXPAND_HORIZONTAL},
    {"arrow-collapse-horizontal", ARROW_COLLAPSE_HORIZONTAL},
    {"stop", STOP},
    {"home", HOME},
    {"home-outline", HOME_OUTLINE},
    {"home-thermometer-outline", HOME_THERMOMETER_OUTLINE},
    {"help-circle-outline", HELP_CIRCLE_OUTLINE},
    {"alert-circle", ALERT_CIRCLE},
    {"alert-circle-outline", ALERT_CIRCLE_OUTLINE},
    {"checkbox-marked-circle", CHECKBOX_MARKED_CIRCLE},
    {"radiobox-blank", RADIOBOX_BLANK},
    {"check-circle", CHECK_CIRCLE},
    {"circle-slice-8", CIRCLE_SLICE_8},
    {"checkbox-blank-circle", CHECKBOX_BLANK_CIRCLE},
    {"lightbulb", LIGHTBULB},
    {"light-switch", LIGHT_SWITCH},
    {"fan", FAN},
    {"script-text", SCRIPT_TEXT},
    {"palette", PALETTE},
    {"gesture-tap-button", GESTURE_TAP_BUTTON},
    {"ray-vertex", RAY_VERTEX},
    {"robot", ROBOT},
    {"robot-vacuum", ROBOT_VACUUM},
    {"account", ACCOUNT},
    {"timer-outline", TIMER_OUTLINE},
    {"timer", TIMER},
    {"thermometer", THERMOMETER},
    {"gauge", GAUGE},
    {"signal", SIGNAL},
    {"smog", SMOG},
    {"flash", FLASH},
    {"cash", CASH},
    {"calendar", CALENDAR},
    {"calendar-clock", CALENDAR_CLOCK},
    {"motion-sensor", MOTION_SENSOR},
    {"motion-sensor-off", MOTION_SENSOR_OFF},
    {"lock", LOCK},
    {"lock-open", LOCK_OPEN},
    {"shield", SHIELD},
    {"shield-off", SHIELD_OFF},
    {"shield-home", SHIELD_HOME},
    {"shield-lock", SHIELD_LOCK},
    {"shield-airplane", SHIELD_AIRPLANE},
    {"shield-moon", SHIELD_MOON},
    {"bell-ring", BELL_RING},
    {"power", POWER},
    {"fire", FIRE},
    {"snowflake", SNOWFLAKE},
    {"water-percent", WATER_PERCENT},
    {"fan-auto", FAN_AUTO},
    {"sun-snowflake-variant", SUN_SNOWFLAKE_VARIANT},
    {"temperature-celsius", TEMPERATURE_CELSIUS},
    {"temperature-fahrenheit", TEMPERATURE_FAHRENHEIT},
    {"window-open", WINDOW_OPEN},
    {"window-closed", WINDOW_CLOSED},
    {"door-open", DOOR_OPEN},
    {"door-closed", DOOR_CLOSED},
    {"garage-open", GARAGE_OPEN},
    {"garage", GARAGE},
    {"blinds-open", BLINDS_OPEN},
    {"blinds", BLINDS},
    {"curtains", CURTAINS},
    {"curtains-closed", CURTAINS_CLOSED},
    {"window-shutter-open", WINDOW_SHUTTER_OPEN},
    {"window-shutter", WINDOW_SHUTTER},
    {"gate-open", GATE_OPEN},
    {"gate", GATE},
    {"battery", BATTERY},
    {"battery-outline", BATTERY_OUTLINE},
    {"battery-charging", BATTERY_CHARGING},
    {"power-plug", POWER_PLUG},
    {"power-plug-off", POWER_PLUG_OFF},
    {"water", WATER},
    {"water-off", WATER_OFF},
    {"smoke-detector", SMOKE_DETECTOR},
    {"smoke-detector-alert", SMOKE_DETECTOR_ALERT},
    {"smoke-detector-variant", SMOKE_DETECTOR_VARIANT},
    {"smoke-detector-variant-alert", SMOKE_DETECTOR_VARIANT_ALERT},
    {"speaker-off", SPEAKER_OFF},
    {"music", MUSIC},
    {"movie", MOVIE},
    {"video", VIDEO},
    {"play", PLAY},
    {"pause", PAUSE},
    {"music-note", MUSIC_NOTE},
    {"music-note-off", MUSIC_NOTE_OFF},
    {"shuffle", SHUFFLE},
    {"shuffle-disabled", SHUFFLE_DISABLE},
    {"weather-sunny", WEATHER_SUNNY},
    {"weather-night", WEATHER_NIGHT},
    {"weather-cloudy", WEATHER_CLOUDY},
    {"weather-partly-cloudy", WEATHER_PARTLY_CLOUDY},
    {"weather-partly-snowy-rainy", WEATHER_PARTLY_SNOWY_RAINY},
    {"weather-fog", WEATHER_FOG},
    {"weather-hail", WEATHER_HAIL},
    {"weather-lightning", WEATHER_LIGHTNING},
    {"weather-lightning-rainy", WEATHER_LIGHTNING_RAINY},
    {"weather-pouring", WEATHER_POURING},
    {"weather-rainy", WEATHER_RAINY},
    {"weather-snowy", WEATHER_SNOWY},
    {"weather-snowy-rainy", WEATHER_SNOWY_RAINY},
    {"weather-windy", WEATHER_WINDY},
    {"weather-windy-variant", WEATHER_WINDY_VARIANT},
    {"weather-sunset-up", WEATHER_SUNSET_UP},
    {"weather-sunset-down", WEATHER_SUNSET_DOWN},
};

}  // namespace

std::array<const char *, 4> cover_icon_set(const std::string &device_class) {
  if (device_class == "awning") return {WINDOW_OPEN, WINDOW_CLOSED, ARROW_UP, ARROW_DOWN};
  if (device_class == "blind") return {BLINDS_OPEN, BLINDS, ARROW_UP, ARROW_DOWN};
  if (device_class == "curtain") return {CURTAINS, CURTAINS_CLOSED, ARROW_EXPAND_HORIZONTAL, ARROW_COLLAPSE_HORIZONTAL};
  if (device_class == "damper") return {CHECKBOX_BLANK_CIRCLE, CIRCLE_SLICE_8, ARROW_UP, ARROW_DOWN};
  if (device_class == "door") return {DOOR_OPEN, DOOR_CLOSED, ARROW_EXPAND_HORIZONTAL, ARROW_COLLAPSE_HORIZONTAL};
  if (device_class == "garage") return {GARAGE_OPEN, GARAGE, ARROW_UP, ARROW_DOWN};
  if (device_class == "gate") return {GATE_OPEN, GATE, ARROW_EXPAND_HORIZONTAL, ARROW_COLLAPSE_HORIZONTAL};
  if (device_class == "shade") return {BLINDS_OPEN, BLINDS, ARROW_UP, ARROW_DOWN};
  if (device_class == "shutter") return {WINDOW_SHUTTER_OPEN, WINDOW_SHUTTER, ARROW_UP, ARROW_DOWN};
  return {WINDOW_OPEN, WINDOW_CLOSED, ARROW_UP, ARROW_DOWN};
}

const char *climate_mode_icon(const std::string &mode) {
  if (mode == "auto" || mode == "heat_cool") return SUN_SNOWFLAKE_VARIANT;
  if (mode == "heat") return FIRE;
  if (mode == "off") return POWER;
  if (mode == "cool") return SNOWFLAKE;
  if (mode == "dry") return WATER_PERCENT;
  if (mode == "fan_only") return FAN;
  return "";
}

std::string resolve_icon(const std::string &value) {
  if (value.rfind("mdi:", 0) == 0) {
    auto key = value.substr(4);
    auto it = NAME_TO_ICON.find(key);
    if (it != NAME_TO_ICON.end()) return it->second;
  }
  return "";
}

const char *icon_for_entity(const std::string &entity_id, const std::string &state,
                            const std::map<std::string, std::string> &attributes) {
  const auto domain = domain_from_entity_id(entity_id);
  const auto device_class = safe_attr(attributes, "device_class");
  const bool is_on = state == "on" || state == "open" || state == "unlocked";

  // Domain-specific defaults
  if (domain == "light") return LIGHTBULB;
  if (domain == "switch") return LIGHT_SWITCH;
  if (domain == "fan") return FAN;
  if (domain == "script") return SCRIPT_TEXT;
  if (domain == "scene") return PALETTE;
  if (domain == "button" || domain == "input_button") return GESTURE_TAP_BUTTON;
  if (domain == "number" || domain == "input_number") return RAY_VERTEX;
  if (domain == "select" || domain == "input_select") return GESTURE_TAP_BUTTON;
  if (domain == "automation") return ROBOT;
  if (domain == "timer") return TIMER_OUTLINE;
  if (domain == "person") return ACCOUNT;
  if (domain == "vacuum") return ROBOT_VACUUM;
  if (domain == "media_player") return state == "off" || state == "idle" || state == "standby" ? SPEAKER_OFF : MUSIC;

  // input_boolean
  if (domain == "input_boolean") return is_on ? CHECK_CIRCLE : RADIOBOX_BLANK;

  // lock
  if (domain == "lock") return is_on ? LOCK_OPEN : LOCK;

  // sun
  if (domain == "sun") return state == "above_horizon" ? WEATHER_SUNNY : WEATHER_NIGHT;

  // alarm_control_panel
  if (domain == "alarm_control_panel") {
    if (state == "disarmed") return SHIELD_OFF;
    if (state == "armed_home") return SHIELD_HOME;
    if (state == "armed_night") return SHIELD_MOON;
    if (state == "armed_away" || state == "armed_vacation") return SHIELD_LOCK;
    if (state == "armed_custom_bypass") return SHIELD_AIRPLANE;
    if (state == "pending" || state == "arming") return BELL_RING;
    if (state == "triggered") return ALERT_CIRCLE;
    return SHIELD;
  }

  // climate
  if (domain == "climate") {
    if (state == "off" || state == "unavailable" || state == "unknown") return POWER;
    if (state == "heat") return FIRE;
    if (state == "cool") return SNOWFLAKE;
    if (state == "heat_cool" || state == "auto") return SUN_SNOWFLAKE_VARIANT;
    if (state == "dry") return WATER_PERCENT;
    if (state == "fan_only") return FAN;
    return THERMOMETER;
  }

  // cover
  if (domain == "cover") {
    const bool is_closed = state == "closed";
    if (device_class == "window") return is_closed ? WINDOW_CLOSED : WINDOW_OPEN;
    if (device_class == "door") return is_closed ? DOOR_CLOSED : DOOR_OPEN;
    if (device_class == "garage") return is_closed ? GARAGE : GARAGE_OPEN;
    if (device_class == "blind") return is_closed ? BLINDS : BLINDS_OPEN;
    if (device_class == "curtain") return is_closed ? CURTAINS_CLOSED : CURTAINS;
    if (device_class == "shutter") return is_closed ? WINDOW_SHUTTER : WINDOW_SHUTTER_OPEN;
    if (device_class == "gate") return is_closed ? GATE : GATE_OPEN;
    return is_closed ? WINDOW_CLOSED : WINDOW_OPEN;
  }

  // binary_sensor
  if (domain == "binary_sensor") {
    if (device_class == "door") return is_on ? DOOR_OPEN : DOOR_CLOSED;
    if (device_class == "window") return is_on ? WINDOW_OPEN : WINDOW_CLOSED;
    if (device_class == "garage_door") return is_on ? GARAGE_OPEN : GARAGE;
    if (device_class == "motion") return is_on ? MOTION_SENSOR : MOTION_SENSOR_OFF;
    if (device_class == "occupancy" || device_class == "presence") return is_on ? ACCOUNT : HOME_OUTLINE;
    if (device_class == "smoke") return is_on ? SMOKE_DETECTOR_ALERT : SMOKE_DETECTOR;
    if (device_class == "moisture") return is_on ? WATER : WATER_OFF;
    if (device_class == "plug") return is_on ? POWER_PLUG : POWER_PLUG_OFF;
    if (device_class == "battery") return is_on ? BATTERY_CHARGING : BATTERY;
    if (device_class == "lock") return is_on ? LOCK_OPEN : LOCK;
    if (device_class == "running") return is_on ? CHECK_CIRCLE : RADIOBOX_BLANK;
    if (device_class == "sound" || device_class == "vibration") return is_on ? ALERT_CIRCLE : RADIOBOX_BLANK;
    return is_on ? CHECKBOX_MARKED_CIRCLE : RADIOBOX_BLANK;
  }

  // sensor
  if (domain == "sensor") {
    if (device_class == "temperature") return THERMOMETER;
    if (device_class == "humidity") return WATER_PERCENT;
    if (device_class == "pressure") return GAUGE;
    if (device_class == "battery") return BATTERY;
    if (device_class == "power" || device_class == "energy" || device_class == "current" || device_class == "voltage") return FLASH;
    if (device_class == "signal_strength") return SIGNAL;
    if (device_class == "timestamp" || device_class == "date") return CALENDAR_CLOCK;
    return THERMOMETER;
  }

  return "";
}

}  // namespace icons
}  // namespace nspanel_lovelace
}  // namespace esphome
