#include "nspanel_lovelace.h"
#include "helpers.h"
#include "icons.h"

namespace esphome {
namespace nspanel_lovelace {

namespace {

const char *const ATTR_CURRENT_TEMPERATURE = "current_temperature";
const char *const ATTR_TEMPERATURE = "temperature";
const char *const ATTR_TARGET_TEMP_HIGH = "target_temp_high";
const char *const ATTR_TARGET_TEMP_LOW = "target_temp_low";
const char *const ATTR_TARGET_TEMP_STEP = "target_temp_step";
const char *const ATTR_MIN_TEMP = "min_temp";
const char *const ATTR_MAX_TEMP = "max_temp";
const char *const ATTR_TEMPERATURE_UNIT = "temperature_unit";
const char *const ATTR_HVAC_MODES = "hvac_modes";
const char *const ATTR_HVAC_MODE = "hvac_mode";
const char *const ATTR_HVAC_ACTION = "hvac_action";
const char *const ATTR_FAN_MODES = "fan_modes";
const char *const ATTR_FAN_MODE = "fan_mode";
const char *const ATTR_SWING_MODES = "swing_modes";
const char *const ATTR_SWING_MODE = "swing_mode";
const char *const ATTR_PRESET_MODES = "preset_modes";
const char *const ATTR_PRESET_MODE = "preset_mode";

static uint16_t thermo_mode_color(const std::string &mode) {
  if (mode == "auto" || mode == "heat_cool") return 0x0400;
  if (mode == "off" || mode == "fan_only") return 0xCE79;
  if (mode == "cool") return 0x2CDF;
  if (mode == "dry") return 0xEDE1;
  return 0xFC00;
}

static void append_thermo_mode_padding(std::string &command, size_t slots) { command.append(4 * slots, '~'); }

static void append_thermo_mode(std::string &command, const std::string &mode, const CardEntity &entity) {
  command.append("~")
      .append(icons::climate_mode_icon(mode)).append("~")
      .append(std::to_string(thermo_mode_color(mode))).append("~")
      .append(entity.state == mode ? "1" : "0").append("~")
      .append(mode);
}

}  // namespace

// --- Card rendering (us -> TFT) ---

void NSPanelLovelace::render_card_thermo_(const CardPage &card) {
  if (card.entities.empty()) {
    return;
  }
  const auto &entity = card.entities[0];

  const auto current_temp = entity.attributes.count(ATTR_CURRENT_TEMPERATURE) ? entity.attributes.at(ATTR_CURRENT_TEMPERATURE) : "";
  const auto unit = entity.attributes.count(ATTR_TEMPERATURE_UNIT) ? entity.attributes.at(ATTR_TEMPERATURE_UNIT) : "°C";

  auto target_temp = entity.attributes.count(ATTR_TEMPERATURE) ? entity.attributes.at(ATTR_TEMPERATURE) : "";
  std::string target_temp_low;
  if (target_temp.empty()) {
    target_temp = entity.attributes.count(ATTR_TARGET_TEMP_HIGH) ? entity.attributes.at(ATTR_TARGET_TEMP_HIGH) : "0";
    target_temp_low = entity.attributes.count(ATTR_TARGET_TEMP_LOW) ? entity.attributes.at(ATTR_TARGET_TEMP_LOW) : "";
    if (!target_temp_low.empty()) {
      float low = 0.0f;
      if (parse_float_(target_temp_low, low)) {
        target_temp_low = std::to_string(static_cast<int>(low * 10));
      }
    }
  }
  int target_temp_raw = 0;
  float target_temp_f = 0.0f;
  if (parse_float_(target_temp, target_temp_f)) {
    target_temp_raw = static_cast<int>(target_temp_f * 10);
  }

  std::string state_text = this->get_translation_(entity.state);
  const auto hvac_action = entity.attributes.count(ATTR_HVAC_ACTION) ? entity.attributes.at(ATTR_HVAC_ACTION) : "";
  if (!hvac_action.empty()) {
    state_text = this->get_translation_(hvac_action).append("\r\n(").append(state_text).append(1, ')');
  }

  int min_temp_raw = 0;
  float min_temp_f = 0.0f;
  if (parse_float_(entity.attributes.count(ATTR_MIN_TEMP) ? entity.attributes.at(ATTR_MIN_TEMP) : "0", min_temp_f)) {
    min_temp_raw = static_cast<int>(min_temp_f * 10);
  }
  int max_temp_raw = 0;
  float max_temp_f = 0.0f;
  if (parse_float_(entity.attributes.count(ATTR_MAX_TEMP) ? entity.attributes.at(ATTR_MAX_TEMP) : "0", max_temp_f)) {
    max_temp_raw = static_cast<int>(max_temp_f * 10);
  }
  int step_raw = 5;
  float step_f = 0.5f;
  if (parse_float_(entity.attributes.count(ATTR_TARGET_TEMP_STEP) ? entity.attributes.at(ATTR_TARGET_TEMP_STEP) : "0.5", step_f)) {
    step_raw = static_cast<int>(step_f * 10);
  }

  std::string command{"entityUpd~"};
  command.append(protocol_escape_(card.title)).append("~");
  this->render_card_navigation_(command);
  command.append("~")
      .append(entity.entity_id).append("~")
      .append(current_temp).append(" ").append(unit).append("~")
      .append(std::to_string(target_temp_raw)).append("~")
      .append(state_text).append("~")
      .append(std::to_string(min_temp_raw)).append("~")
      .append(std::to_string(max_temp_raw)).append("~")
      .append(std::to_string(step_raw));

  const auto hvac_modes_str = entity.attributes.count(ATTR_HVAC_MODES) ? entity.attributes.at(ATTR_HVAC_MODES) : "";
  if (hvac_modes_str.empty()) {
    command.append(32, '~');
  } else {
    auto modes = split_(hvac_modes_str, ',');
    const size_t count = modes.size() > 8 ? 8 : modes.size();
    const bool compact_layout = this->model_ == "us-p" || count >= 5;
    if (compact_layout) {
      for (size_t i = 0; i < count; i++) append_thermo_mode(command, modes[i], entity);
      append_thermo_mode_padding(command, 8 - count);
    } else if (count == 1) {
      append_thermo_mode_padding(command, 1);
      append_thermo_mode(command, modes[0], entity);
      append_thermo_mode_padding(command, 6);
    } else if (count == 2) {
      append_thermo_mode_padding(command, 2);
      append_thermo_mode(command, modes[0], entity);
      append_thermo_mode_padding(command, 2);
      append_thermo_mode(command, modes[1], entity);
      append_thermo_mode_padding(command, 2);
    } else if (count == 3) {
      append_thermo_mode_padding(command, 2);
      append_thermo_mode(command, modes[0], entity);
      append_thermo_mode_padding(command, 1);
      append_thermo_mode(command, modes[1], entity);
      append_thermo_mode_padding(command, 1);
      append_thermo_mode(command, modes[2], entity);
      append_thermo_mode_padding(command, 1);
    } else if (count == 4) {
      append_thermo_mode_padding(command, 1);
      append_thermo_mode(command, modes[0], entity);
      append_thermo_mode_padding(command, 1);
      append_thermo_mode(command, modes[1], entity);
      append_thermo_mode_padding(command, 1);
      append_thermo_mode(command, modes[2], entity);
      append_thermo_mode_padding(command, 1);
      append_thermo_mode(command, modes[3], entity);
    }
  }

  const auto temp_unit_icon = unit == "°F" || unit == "F" ? icons::TEMPERATURE_FAHRENHEIT : icons::TEMPERATURE_CELSIUS;
  const bool has_detail = entity.attributes.count(ATTR_PRESET_MODES) || entity.attributes.count(ATTR_SWING_MODES) ||
                          entity.attributes.count(ATTR_FAN_MODES);
  command.append("~~")
      .append(this->get_translation_("currently")).append("~")
      .append(this->get_translation_("state")).append("~~")
      .append(temp_unit_icon).append("~")
      .append(target_temp_low).append("~")
      .append(has_detail ? "0" : "1");

  this->send_display_command(std::move(command));
}

}  // namespace nspanel_lovelace
}  // namespace esphome
