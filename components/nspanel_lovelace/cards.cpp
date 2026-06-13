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
const char *const ATTR_MIN = "min";
const char *const ATTR_MAX = "max";
const char *const ATTR_STEP = "step";
const char *const ATTR_TEMPERATURE = "temperature";
const char *const ATTR_CURRENT_TEMPERATURE = "current_temperature";
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

void NSPanelLovelace::add_card_entities(std::string type, std::string title) {
  this->cards_.push_back(CardPage{std::move(type), std::move(title), "", {}});
}

void NSPanelLovelace::add_card_qr(std::string title, std::string qr_text) {
  this->cards_.push_back(CardPage{"cardQR", std::move(title), std::move(qr_text), {}});
}

void NSPanelLovelace::add_card_thermo(std::string title, std::string entity_id) {
  this->cards_.push_back(CardPage{"cardThermo", std::move(title), "", {CardEntity{std::move(entity_id), "", "", 17299, "", {}}}});
}

void NSPanelLovelace::add_card_entity(std::string entity_id, std::string name, std::string icon, uint16_t color) {
  if (this->cards_.empty()) {
    return;
  }
  if (name.empty()) {
    name = entity_id;
  }
  this->cards_.back().entities.push_back(CardEntity{std::move(entity_id), std::move(name), std::move(icon), color, "", {}});
}

void NSPanelLovelace::subscribe_card_entities_() {
  for (const auto &card : this->cards_) {
    for (const auto &entity : card.entities) {
      this->subscribe_homeassistant_state(&NSPanelLovelace::on_card_entity_state_, entity.entity_id);
      const auto domain = entity_domain_(entity.entity_id);
      if (domain == "light") {
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_BRIGHTNESS);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_COLOR_TEMP);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_COLOR_MODE);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_SUPPORTED_COLOR_MODES);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_MIN_MIREDS);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_MAX_MIREDS);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_EFFECT_LIST);
      } else if (domain == "cover") {
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_CURRENT_POSITION);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_CURRENT_TILT_POSITION);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_SUPPORTED_FEATURES);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_DEVICE_CLASS);
      } else if (domain == "fan") {
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_PERCENTAGE);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_PERCENTAGE_STEP);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_PRESET_MODE);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_PRESET_MODES);
      } else if (domain == "select" || domain == "input_select") {
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_OPTIONS);
      } else if (domain == "number" || domain == "input_number") {
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_MIN);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_MAX);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_STEP);
      } else if (domain == "climate") {
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_TEMPERATURE);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_CURRENT_TEMPERATURE);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_TARGET_TEMP_HIGH);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_TARGET_TEMP_LOW);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_TARGET_TEMP_STEP);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_MIN_TEMP);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_MAX_TEMP);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_TEMPERATURE_UNIT);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_HVAC_MODES);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_HVAC_MODE);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_HVAC_ACTION);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_PRESET_MODES);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_PRESET_MODE);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_FAN_MODES);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_FAN_MODE);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_SWING_MODES);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_SWING_MODE);
      }
    }
  }
}

CardEntity *NSPanelLovelace::find_card_entity_(const std::string &entity_id) {
  for (auto &card : this->cards_) {
    for (auto &entity : card.entities) {
      if (entity.entity_id == entity_id) {
        return &entity;
      }
    }
  }
  return nullptr;
}

void NSPanelLovelace::on_card_entity_state_(const std::string &entity_id, StringRef state) {
  auto entity = this->find_card_entity_(entity_id);
  if (entity == nullptr) {
    return;
  }
  entity->state = state.str();
  this->render_current_card_();
  if (!this->popup_entity_id_.empty() && this->popup_entity_id_ == entity_id) {
    this->render_popup_();
  }
}

void NSPanelLovelace::on_card_entity_attr_(const std::string &entity_id, const std::string &attr, StringRef value) {
  auto entity = this->find_card_entity_(entity_id);
  if (entity == nullptr) {
    return;
  }
  entity->attributes[attr] = value.str();
  if (!this->popup_entity_id_.empty() && this->popup_entity_id_ == entity_id) {
    this->render_popup_();
  }
}

void NSPanelLovelace::show_card_(size_t index) {
  if (index >= this->cards_.size()) {
    return;
  }
  this->current_card_ = index;
  this->card_visible_ = true;
  this->close_popup_();
  this->send_display_command("pageType~" + this->cards_[index].type);
  this->apply_display_settings_();
  this->render_current_card_();
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
  if (card.type == "cardThermo") {
    this->render_card_thermo_(card);
    return;
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
      .append("~").append(icons::ARROW_LEFT_BOLD).append("~65535~~")
      .append("~button~navigate.uuid.")
      .append(std::to_string(next))
      .append("~").append(icons::ARROW_RIGHT_BOLD).append("~65535~~");
}

void NSPanelLovelace::append_card_entity_(std::string &command, const CardEntity &entity) const {
  std::string icon = entity.icon.empty()
                         ? icons::icon_for_entity(entity.entity_id, entity.state, entity.attributes)
                         : icons::resolve_icon(entity.icon);
  command.append("~")
      .append(entity_render_type_(entity.entity_id))
      .append("~")
      .append(protocol_escape_(entity.entity_id))
      .append("~")
      .append(protocol_escape_(icon))
      .append("~")
      .append(std::to_string(entity.color))
      .append("~")
      .append(protocol_escape_(entity.name))
      .append("~")
      .append(protocol_escape_(this->entity_value_(entity)));
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
  if (domain == "climate") {
    return "text";
  }
  return "text";
}

std::string NSPanelLovelace::entity_value_(const CardEntity &entity) const {
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
  if (domain == "number" || domain == "input_number") {
    auto min_val = entity.attributes.count(ATTR_MIN) ? entity.attributes.at(ATTR_MIN) : "0";
    auto max_val = entity.attributes.count(ATTR_MAX) ? entity.attributes.at(ATTR_MAX) : "100";
    return entity.state + "|" + min_val + "|" + max_val;
  }
  if (domain == "climate") {
    auto temp = entity.attributes.count(ATTR_TEMPERATURE) ? entity.attributes.at(ATTR_TEMPERATURE) : "";
    auto current = entity.attributes.count(ATTR_CURRENT_TEMPERATURE) ? entity.attributes.at(ATTR_CURRENT_TEMPERATURE) : "";
    auto unit = entity.attributes.count(ATTR_TEMPERATURE_UNIT) ? entity.attributes.at(ATTR_TEMPERATURE_UNIT) : "°C";
    std::string value = this->get_translation_(entity.state);
    if (!temp.empty()) {
      value.append(" ").append(temp).append(unit);
    }
    value.append("\r\n").append(this->get_translation_("current_temperature")).append(": ").append(current).append(unit);
    return value;
  }
  return entity.state;
}

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
    command.append(32, '~');  // 8 empty mode slots
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

void NSPanelLovelace::show_popup_(const std::string &page_type, const std::string &entity_id) {
  auto entity = this->find_card_entity_(entity_id);
  if (entity == nullptr) {
    return;
  }
  this->popup_entity_id_ = entity_id;
  this->popup_page_type_ = page_type;
  this->card_visible_ = false;
  this->send_display_command("pageType~" + page_type);
  this->send_display_command("timeout~10");
  this->render_popup_();
}

void NSPanelLovelace::close_popup_() {
  this->popup_entity_id_.clear();
  this->popup_page_type_.clear();
}

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
      .append(cover_icons[2]).append("~")  // icon_up
      .append(icons::STOP).append("~")    // icon_stop
      .append(cover_icons[3]).append("~")  // icon_down
      .append(icon_up_status ? "enable" : "disable").append("~")
      .append(icon_stop_status ? "enable" : "disable").append("~")
      .append(icon_down_status ? "enable" : "disable").append("~")
      .append(text_tilt).append("~")
      .append(icons::ARROW_TOP_RIGHT).append("~")    // icon_tilt_left
      .append(icons::STOP).append("~")               // icon_tilt_stop
      .append(icons::ARROW_BOTTOM_LEFT).append("~")  // icon_tilt_right
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
  auto preset_modes = entity.attributes.count(ATTR_PRESET_MODES) ? entity.attributes.at(ATTR_PRESET_MODES) : "";

  for (auto &c : preset_modes) {
    if (c == ',') c = '?';
  }

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
  auto options = entity.attributes.count(ATTR_OPTIONS) ? entity.attributes.at(ATTR_OPTIONS) : "";
  for (auto &c : options) {
    if (c == ',') c = '?';
  }

  std::string command{"entityUpdateDetail2~"};
  command.append(entity.entity_id).append(2, '~')
      .append(std::to_string(entity.color)).append("~")
      .append(entity_domain_(entity.entity_id)).append("~")
      .append(entity.state).append("~")
      .append(options).append("~");
  this->send_display_command(std::move(command));
}

void NSPanelLovelace::render_climate_detail_(const CardEntity &entity) {
  uint16_t icon_color = 64512;
  if (entity.state == "auto" || entity.state == "heat_cool") {
    icon_color = 1024;
  } else if (entity.state == "off" || entity.state == "fan_only") {
    icon_color = 35921;
  } else if (entity.state == "cool") {
    icon_color = 11487;
  } else if (entity.state == "dry") {
    icon_color = 60897;
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
      {ATTR_PRESET_MODES, ATTR_PRESET_MODE, "preset", true},
      {ATTR_SWING_MODES, ATTR_SWING_MODE, "swing", false},
      {ATTR_FAN_MODES, ATTR_FAN_MODE, "fan_only", false},
  };

  for (const auto &section : sections) {
    if (!entity.attributes.count(section.attr)) continue;
    const auto &modes_str = entity.attributes.at(section.attr);
    if (modes_str.empty()) continue;

    std::string modes_res;
    if (section.translate_items) {
      size_t start = 0;
      while (start <= modes_str.size()) {
        auto end = modes_str.find(',', start);
        auto part = modes_str.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (!modes_res.empty()) modes_res.push_back('?');
        modes_res.append(this->get_translation_(part));
        if (end == std::string::npos) break;
        start = end + 1;
      }
    } else {
      modes_res = modes_str;
      std::replace(modes_res.begin(), modes_res.end(), ',', '?');
    }

    command.append(this->get_translation_(section.heading_key)).append("~")
        .append(section.attr).append("~")
        .append(entity.attributes.count(section.current_attr) ? entity.attributes.at(section.current_attr) : "").append("~")
        .append(modes_res).append("~");
  }

  this->send_display_command(std::move(command));
}

}  // namespace nspanel_lovelace
}  // namespace esphome
