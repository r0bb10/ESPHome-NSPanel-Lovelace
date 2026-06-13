#include "nspanel_lovelace.h"

namespace esphome {
namespace nspanel_lovelace {

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
  this->cards_.back().entities.push_back(CardEntity{std::move(entity_id), std::move(name), std::move(icon), color, ""});
}

void NSPanelLovelace::subscribe_card_entities_() {
  for (const auto &card : this->cards_) {
    for (const auto &entity : card.entities) {
      this->subscribe_homeassistant_state(&NSPanelLovelace::on_card_entity_state_, entity.entity_id);
    }
  }
}

void NSPanelLovelace::on_card_entity_state_(const std::string &entity_id, StringRef state) {
  for (auto &card : this->cards_) {
    for (auto &entity : card.entities) {
      if (entity.entity_id == entity_id) {
        entity.state = state.str();
      }
    }
  }
  this->render_current_card_();
}

void NSPanelLovelace::show_card_(size_t index) {
  if (index >= this->cards_.size()) {
    return;
  }
  this->current_card_ = index;
  this->card_visible_ = true;
  this->send_display_command("pageType~" + this->cards_[index].type);
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

}  // namespace nspanel_lovelace
}  // namespace esphome
