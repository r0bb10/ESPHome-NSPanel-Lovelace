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
const char *const ATTR_CODE_ARM_REQUIRED = "code_arm_required";
const char *const ATTR_OPEN_SENSORS = "open_sensors";

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

void NSPanelLovelace::add_card_alarm(std::string title, std::string entity_id, std::vector<std::string> supported_modes) {
  this->cards_.push_back(
      CardPage{"cardAlarm", std::move(title), "", {CardEntity{std::move(entity_id), "", "", 17299, "", {}}}, std::move(supported_modes)});
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
      } else if (domain == "alarm_control_panel") {
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_CODE_ARM_REQUIRED);
        this->subscribe_homeassistant_state_attr_(entity.entity_id, ATTR_OPEN_SENSORS);
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
  if (card.type == "cardAlarm") {
    this->render_card_alarm_(card);
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

}  // namespace nspanel_lovelace
}  // namespace esphome
