#include "nspanel_lovelace.h"
#include "helpers.h"
#include "icons.h"

namespace esphome {
namespace nspanel_lovelace {

namespace {

const char *const ATTR_BRIGHTNESS = "brightness";
const char *const ATTR_COLOR_TEMP = "color_temp";
const char *const ATTR_COLOR_MODE = "color_mode";
const char *const ATTR_SUPPORTED_COLOR_MODES = "supported_color_modes";
const char *const ATTR_MIN_MIREDS = "min_mireds";
const char *const ATTR_MAX_MIREDS = "max_mireds";
const char *const ATTR_EFFECT_LIST = "effect_list";
const char *const ATTR_CURRENT_POSITION = "current_position";
const char *const ATTR_CURRENT_TILT_POSITION = "current_tilt_position";
const char *const ATTR_SUPPORTED_FEATURES = "supported_features";
const char *const ATTR_DEVICE_CLASS = "device_class";
const char *const ATTR_PERCENTAGE = "percentage";
const char *const ATTR_PERCENTAGE_STEP = "percentage_step";
const char *const ATTR_PRESET_MODE = "preset_mode";
const char *const ATTR_PRESET_MODES = "preset_modes";
const char *const ATTR_OPTIONS = "options";
const char *const ATTR_FAN_MODES = "fan_modes";
const char *const ATTR_FAN_MODE = "fan_mode";
const char *const ATTR_SWING_MODES = "swing_modes";
const char *const ATTR_SWING_MODE = "swing_mode";

static std::string join_modes_(const std::vector<std::string> &modes) {
  std::string result;
  for (const auto &mode : modes) {
    if (!result.empty()) result.push_back('?');
    result.append(mode);
  }
  return result;
}

}  // namespace

// --- Card rendering (us -> TFT) ---

void NSPanelLovelace::render_popup_() {
  if (this->popup_entity_id_.empty()) {
    return;
  }
  auto entity = this->find_card_entity_(this->popup_entity_id_);
  if (entity == nullptr) {
    return;
  }
  const auto domain = entity_domain_(entity->entity_id);
  if (domain == "light") {
    this->render_light_detail_(*entity);
  } else if (domain == "cover") {
    this->render_cover_detail_(*entity);
  } else if (domain == "fan") {
    this->render_fan_detail_(*entity);
  } else if (domain == "select" || domain == "input_select") {
    this->render_select_detail_(*entity);
  } else if (domain == "climate") {
    this->render_climate_detail_(*entity);
  }
}

// --- Detail page renderers ---

void NSPanelLovelace::render_light_detail_(const CardEntity &entity) {
  const auto &supported_modes = entity.attributes.count(ATTR_SUPPORTED_COLOR_MODES)
                                    ? entity.attributes.at(ATTR_SUPPORTED_COLOR_MODES)
                                    : "";
  bool enable_color_wheel = entity.state == "on" &&
                            (contains_value(supported_modes, "xy") || contains_value(supported_modes, "hs") ||
                             contains_value(supported_modes, "rgb") || contains_value(supported_modes, "rgbw") ||
                             contains_value(supported_modes, "rgbww"));

  const auto &color_mode = entity.attributes.count(ATTR_COLOR_MODE) ? entity.attributes.at(ATTR_COLOR_MODE) : "";
  std::string color_temp = "disable";
  if (contains_value(supported_modes, "color_temp")) {
    if (color_mode == "color_temp") {
      if (entity.attributes.count(ATTR_COLOR_TEMP) && entity.attributes.count(ATTR_MIN_MIREDS) &&
          entity.attributes.count(ATTR_MAX_MIREDS)) {
        int mireds = 0, min_mireds = 0, max_mireds = 0;
        parse_int_(entity.attributes.at(ATTR_COLOR_TEMP), mireds);
        parse_int_(entity.attributes.at(ATTR_MIN_MIREDS), min_mireds);
        parse_int_(entity.attributes.at(ATTR_MAX_MIREDS), max_mireds);
        if (mireds > 0 && min_mireds > 0 && max_mireds > 0 && min_mireds < max_mireds) {
          auto min_kelvin = 1000000.0 / max_mireds;
          auto max_kelvin = 1000000.0 / min_mireds;
          auto kelvin = 1000000.0 / mireds;
          color_temp = std::to_string(static_cast<int>(scale_value(kelvin, {max_kelvin, min_kelvin}, {0, 100})));
        }
      }
      if (color_temp == "disable") {
        color_temp = "unknown";
      }
    } else {
      color_temp = "unknown";
    }
  }

  std::string brightness = "disable";
  if (entity.attributes.count(ATTR_BRIGHTNESS)) {
    int ha_brightness = 0;
    if (parse_int_(entity.attributes.at(ATTR_BRIGHTNESS), ha_brightness) && ha_brightness > 0) {
      brightness = std::to_string(static_cast<int>(scale_value(ha_brightness, {0, 255}, {0, 100})));
    }
  }

  std::string command{"entityUpdateDetail~"};
  command.append(entity.entity_id).append(2, '~')
      .append(std::to_string(entity.color)).append("~")
      .append(entity.state == "on" ? "1" : "0").append("~")
      .append(brightness).append("~")
      .append(color_temp).append("~")
      .append(enable_color_wheel ? "enable" : "disable").append("~")
      .append(this->get_translation_("color")).append("~")
      .append(this->get_translation_("color_temp")).append("~")
      .append(this->get_translation_("brightness")).append("~")
      .append(entity.attributes.count(ATTR_EFFECT_LIST) ? "enable" : "disable");
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::render_cover_detail_(const CardEntity &entity) {
  uint16_t supported_features = 0;
  if (entity.attributes.count(ATTR_SUPPORTED_FEATURES)) {
    parse_int_(entity.attributes.at(ATTR_SUPPORTED_FEATURES), supported_features);
  }

  uint8_t position = 0;
  if (entity.attributes.count(ATTR_CURRENT_POSITION)) {
    parse_int_(entity.attributes.at(ATTR_CURRENT_POSITION), position);
  }
  uint8_t tilt_position = 0;
  if (entity.attributes.count(ATTR_CURRENT_TILT_POSITION)) {
    parse_int_(entity.attributes.at(ATTR_CURRENT_TILT_POSITION), tilt_position);
  }

  bool position_status = (supported_features & 0b00001111) != 0;
  bool icon_up_status = false;
  bool icon_down_status = false;
  bool icon_stop_status = false;
  bool icon_tilt_left_status = false;
  bool icon_tilt_right_status = false;
  bool icon_tilt_stop_status = false;
  bool tilt_position_status = false;

  if (supported_features & 0b00000001) {
    icon_up_status = position != 100 && !((entity.state == "open" || entity.state == "unknown") &&
                                          !entity.attributes.count(ATTR_CURRENT_POSITION));
  }
  if (supported_features & 0b00000010) {
    icon_down_status = position != 0 && !((entity.state == "closed" || entity.state == "unknown") &&
                                          !entity.attributes.count(ATTR_CURRENT_POSITION));
  }
  if (supported_features & 0b00001000) {
    icon_stop_status = entity.state != "unknown";
  }

  std::string text_tilt;
  if (supported_features & 0b11110000) {
    text_tilt = this->get_translation_("tilt_position");
  }
  if (supported_features & 0b00010000) {
    icon_tilt_left_status = true;
  }
  if (supported_features & 0b00100000) {
    icon_tilt_right_status = true;
  }
  if (supported_features & 0b01000000) {
    icon_tilt_stop_status = true;
  }
  if (supported_features & 0b10000000) {
    tilt_position_status = true;
    if (tilt_position == 0) icon_tilt_right_status = false;
    if (tilt_position == 100) icon_tilt_left_status = false;
  }

  const auto device_class = entity.attributes.count(ATTR_DEVICE_CLASS) ? entity.attributes.at(ATTR_DEVICE_CLASS) : "";
  const auto cover_icons = icons::cover_icon_set(device_class);
  const char *main_icon = entity.state == "closed" ? cover_icons[1] : cover_icons[0];

  std::string command{"entityUpdateDetail~"};
  command.append(entity.entity_id).append("~")
      .append(std::to_string(position)).append("~")
      .append(position_status ? std::to_string(position).append("%") : entity.state).append("~")
      .append(this->get_translation_("position")).append("~")
      .append(main_icon).append("~")
      .append(cover_icons[2]).append("~")
      .append(icons::STOP).append("~")
      .append(cover_icons[3]).append("~")
      .append(icon_up_status ? "enable" : "disable").append("~")
      .append(icon_stop_status ? "enable" : "disable").append("~")
      .append(icon_down_status ? "enable" : "disable").append("~")
      .append(text_tilt).append("~")
      .append(icons::ARROW_TOP_RIGHT).append("~")
      .append(icons::STOP).append("~")
      .append(icons::ARROW_BOTTOM_LEFT).append("~")
      .append(icon_tilt_left_status ? "enable" : "disable").append("~")
      .append(icon_tilt_stop_status ? "enable" : "disable").append("~")
      .append(icon_tilt_right_status ? "enable" : "disable").append("~")
      .append(tilt_position_status ? std::to_string(tilt_position) : "");
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::render_fan_detail_(const CardEntity &entity) {
  auto speed = entity.attributes.count(ATTR_PERCENTAGE) ? entity.attributes.at(ATTR_PERCENTAGE) : "";
  auto percentage_step = entity.attributes.count(ATTR_PERCENTAGE_STEP) ? entity.attributes.at(ATTR_PERCENTAGE_STEP) : "";
  auto preset_mode = entity.attributes.count(ATTR_PRESET_MODE) ? entity.attributes.at(ATTR_PRESET_MODE) : "";
  auto preset_modes = entity.attributes.count(ATTR_PRESET_MODES) ? join_modes_(split_list_attr_(entity.attributes.at(ATTR_PRESET_MODES))) : "";

  uint8_t speed_max = 100;
  if (!percentage_step.empty()) {
    float speed_val = 0.0f;
    float step_val = 1.0f;
    if (speed.empty()) {
      speed = "0";
    } else if (!parse_float_(speed, speed_val)) {
      speed_val = 0.0f;
    }
    if (!parse_float_(percentage_step, step_val)) {
      step_val = 1.0f;
    }
    if (step_val < 1.0f) step_val = 1.0f;
    int steps = static_cast<int>(round(speed_val / step_val));
    speed = std::to_string(steps);
    speed_max = static_cast<uint8_t>(round(100.0f / step_val));
  }

  std::string command{"entityUpdateDetail~"};
  command.append(entity.entity_id).append(2, '~')
      .append(std::to_string(entity.color)).append("~")
      .append(entity.state == "on" ? "1" : "0").append("~")
      .append(percentage_step.empty() ? "disable" : speed).append("~")
      .append(std::to_string(speed_max)).append("~")
      .append(this->get_translation_("speed")).append("~")
      .append(preset_mode).append("~")
      .append(preset_modes);
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::render_select_detail_(const CardEntity &entity) {
  auto options = entity.attributes.count(ATTR_OPTIONS) ? join_modes_(split_list_attr_(entity.attributes.at(ATTR_OPTIONS))) : "";

  std::string command{"entityUpdateDetail2~"};
  command.append(entity.entity_id).append(2, '~')
      .append(std::to_string(entity.color)).append("~")
      .append(entity_domain_(entity.entity_id)).append("~")
      .append(entity.state).append("~")
      .append(options).append("~");
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::render_climate_detail_(const CardEntity &entity) {
  uint16_t icon_color = 0xFC00;
  if (entity.state == "auto" || entity.state == "heat_cool") {
    icon_color = 0x0400;
  } else if (entity.state == "off" || entity.state == "fan_only") {
    icon_color = 0x8C51;
  } else if (entity.state == "cool") {
    icon_color = 0x2CDF;
  } else if (entity.state == "dry") {
    icon_color = 0xEDE1;
  }

  std::string command{"entityUpdateDetail~"};
  command.append(entity.entity_id).append("~")
      .append(icons::icon_for_entity(entity.entity_id, entity.state, entity.attributes)).append("~")
      .append(std::to_string(icon_color)).append("~");

  struct ModeSection {
    const char *attr;
    const char *current_attr;
    const char *heading_key;
    bool translate_items;
  };
  static const ModeSection sections[] = {
      {ATTR_PRESET_MODES, ATTR_PRESET_MODE, "preset_mode", true},
      {ATTR_SWING_MODES, ATTR_SWING_MODE, "swing_mode", false},
      {ATTR_FAN_MODES, ATTR_FAN_MODE, "fan_mode", false},
  };

  for (const auto &section : sections) {
    if (!entity.attributes.count(section.attr)) continue;
    const auto &modes_str = entity.attributes.at(section.attr);
    if (modes_str.empty()) continue;

    std::string modes_res;
    std::string current = entity.attributes.count(section.current_attr) ? entity.attributes.at(section.current_attr) : "";
    if (section.translate_items) {
      const auto modes = split_list_attr_(modes_str);
      for (const auto &part : modes) {
        if (!modes_res.empty()) modes_res.push_back('?');
        modes_res.append(this->get_translation_(part));
      }
      current = this->get_translation_(current);
    } else {
      const auto modes = split_list_attr_(modes_str);
      for (const auto &mode : modes) {
        if (!modes_res.empty()) modes_res.push_back('?');
        modes_res.append(mode);
      }
    }

    command.append(this->get_translation_(section.heading_key)).append("~")
        .append(section.attr).append("~")
        .append(current).append("~")
        .append(modes_res).append("~");
  }

  this->send_display_command(std::move(command));
}

}  // namespace nspanel_lovelace
}  // namespace esphome
