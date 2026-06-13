#pragma once

#include <array>
#include <map>
#include <string>

namespace esphome {
namespace nspanel_lovelace {
namespace icons {

// Common / navigation
extern const char *const ARROW_LEFT_BOLD;
extern const char *const ARROW_RIGHT_BOLD;
extern const char *const ARROW_UP;
extern const char *const ARROW_DOWN;
extern const char *const ARROW_UP_BOLD;
extern const char *const ARROW_TOP_RIGHT;
extern const char *const ARROW_BOTTOM_LEFT;
extern const char *const ARROW_EXPAND_HORIZONTAL;
extern const char *const ARROW_COLLAPSE_HORIZONTAL;
extern const char *const STOP;
extern const char *const HOME;
extern const char *const HOME_OUTLINE;
extern const char *const HELP_CIRCLE_OUTLINE;
extern const char *const ALERT_CIRCLE;
extern const char *const ALERT_CIRCLE_OUTLINE;
extern const char *const CHECKBOX_MARKED_CIRCLE;
extern const char *const RADIOBOX_BLANK;
extern const char *const CHECK_CIRCLE;
extern const char *const CIRCLE_SLICE_8;
extern const char *const CHECKBOX_BLANK_CIRCLE;

// Entity type defaults
extern const char *const LIGHTBULB;
extern const char *const LIGHT_SWITCH;
extern const char *const FAN;
extern const char *const SCRIPT_TEXT;
extern const char *const PALETTE;
extern const char *const GESTURE_TAP_BUTTON;
extern const char *const RAY_VERTEX;
extern const char *const ROBOT;
extern const char *const ROBOT_VACUUM;
extern const char *const ACCOUNT;
extern const char *const TIMER_OUTLINE;
extern const char *const TIMER;
extern const char *const THERMOMETER;
extern const char *const GAUGE;
extern const char *const SIGNAL;
extern const char *const SMOG;
extern const char *const FLASH;
extern const char *const CASH;
extern const char *const CALENDAR;
extern const char *const CALENDAR_CLOCK;
extern const char *const MOTION_SENSOR;
extern const char *const MOTION_SENSOR_OFF;

// Lock / security
extern const char *const LOCK;
extern const char *const LOCK_OPEN;
extern const char *const SHIELD;
extern const char *const SHIELD_OFF;
extern const char *const SHIELD_HOME;
extern const char *const SHIELD_LOCK;
extern const char *const SHIELD_AIRPLANE;
extern const char *const SHIELD_MOON;
extern const char *const BELL_RING;

// Climate / power
extern const char *const POWER;
extern const char *const FIRE;
extern const char *const SNOWFLAKE;
extern const char *const WATER_PERCENT;
extern const char *const FAN_AUTO;
extern const char *const SUN_SNOWFLAKE_VARIANT;
extern const char *const TEMPERATURE_CELSIUS;
extern const char *const TEMPERATURE_FAHRENHEIT;

// Cover
extern const char *const WINDOW_OPEN;
extern const char *const WINDOW_CLOSED;
extern const char *const DOOR_OPEN;
extern const char *const DOOR_CLOSED;
extern const char *const GARAGE_OPEN;
extern const char *const GARAGE;
extern const char *const BLINDS_OPEN;
extern const char *const BLINDS;
extern const char *const CURTAINS;
extern const char *const CURTAINS_CLOSED;
extern const char *const WINDOW_SHUTTER_OPEN;
extern const char *const WINDOW_SHUTTER;
extern const char *const GATE_OPEN;
extern const char *const GATE;

// Battery / plug
extern const char *const BATTERY;
extern const char *const BATTERY_OUTLINE;
extern const char *const BATTERY_CHARGING;
extern const char *const POWER_PLUG;
extern const char *const POWER_PLUG_OFF;

// Water / moisture
extern const char *const WATER;
extern const char *const WATER_OFF;

// Smoke / fire safety
extern const char *const SMOKE_DETECTOR;
extern const char *const SMOKE_DETECTOR_ALERT;
extern const char *const SMOKE_DETECTOR_VARIANT;
extern const char *const SMOKE_DETECTOR_VARIANT_ALERT;

// Media
extern const char *const SPEAKER_OFF;
extern const char *const MUSIC;
extern const char *const MOVIE;
extern const char *const VIDEO;
extern const char *const PLAY;
extern const char *const PAUSE;
extern const char *const MUSIC_NOTE;
extern const char *const MUSIC_NOTE_OFF;

// Weather
extern const char *const WEATHER_SUNNY;
extern const char *const WEATHER_NIGHT;
extern const char *const WEATHER_CLOUDY;
extern const char *const WEATHER_PARTLY_CLOUDY;
extern const char *const WEATHER_PARTLY_SNOWY_RAINY;
extern const char *const WEATHER_FOG;
extern const char *const WEATHER_HAIL;
extern const char *const WEATHER_LIGHTNING;
extern const char *const WEATHER_LIGHTNING_RAINY;
extern const char *const WEATHER_POURING;
extern const char *const WEATHER_RAINY;
extern const char *const WEATHER_SNOWY;
extern const char *const WEATHER_SNOWY_RAINY;
extern const char *const WEATHER_WINDY;
extern const char *const WEATHER_WINDY_VARIANT;
extern const char *const WEATHER_SUNSET_UP;
extern const char *const WEATHER_SUNSET_DOWN;

// Resolves an mdi icon name (e.g. "mdi:lightbulb") to a Nextion character.
std::string resolve_icon(const std::string &value);

// Selects a default icon for an entity based on domain, state, and device_class.
const char *icon_for_entity(const std::string &entity_id, const std::string &state,
                            const std::map<std::string, std::string> &attributes);

// Returns the four cover detail icons for a device_class: {open, closed, up, down}.
// Falls back to window icons if the device_class is unknown.
std::array<const char *, 4> cover_icon_set(const std::string &device_class);

// Returns the icon for a climate hvac mode (auto, heat_cool, heat, off, cool, dry, fan_only).
const char *climate_mode_icon(const std::string &mode);

}  // namespace icons
}  // namespace nspanel_lovelace
}  // namespace esphome
