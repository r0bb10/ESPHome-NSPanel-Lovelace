#include "nspanel_lovelace.h"

#include "helpers.h"

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

}  // namespace

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
      color_temp = entity.attributes.count(ATTR_COLOR_TEMP) ? entity.attributes.at(ATTR_COLOR_TEMP) : "disable";
    } else {
      color_temp = "unknown";
    }
  } else {
    color_temp = "disable";
  }

  std::string brightness = "disable";
  if (entity.attributes.count(ATTR_BRIGHTNESS)) {
    brightness = entity.attributes.at(ATTR_BRIGHTNESS);
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

  std::string command{"entityUpdateDetail~"};
  command.append(entity.entity_id).append("~")
      .append(std::to_string(position)).append("~")
      .append(position_status ? std::to_string(position).append("%") : entity.state).append("~")
      .append(this->get_translation_("position")).append("~")
      .append("").append("~")  // icon
      .append("").append("~")  // icon_up
      .append("").append("~")  // icon_stop
      .append("").append("~")  // icon_down
      .append(icon_up_status ? "enable" : "disable").append("~")
      .append(icon_stop_status ? "enable" : "disable").append("~")
      .append(icon_down_status ? "enable" : "disable").append("~")
      .append(text_tilt).append("~")
      .append("").append("~")  // icon_tilt_left
      .append("").append("~")  // icon_tilt_stop
      .append("").append("~")  // icon_tilt_right
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

}  // namespace nspanel_lovelace
}  // namespace esphome
