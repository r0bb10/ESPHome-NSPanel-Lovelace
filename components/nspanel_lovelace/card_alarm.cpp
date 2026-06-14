#include "nspanel_lovelace.h"
#include "icons.h"

namespace esphome {
namespace nspanel_lovelace {

namespace {

const char *const ATTR_OPEN_SENSORS = "open_sensors";
const char *const ATTR_SUPPORTED_FEATURES = "supported_features";

}  // namespace

// --- Card rendering (us -> TFT) ---

void NSPanelLovelace::render_card_alarm_(const CardPage &card) {
  if (card.entities.empty()) {
    return;
  }
  const auto &entity = card.entities[0];

  auto alarm_icon = icons::SHIELD_OFF;
  uint16_t alarm_color = 0x0CE6;
  if (entity.state == "armed_home") {
    alarm_icon = icons::SHIELD_HOME;
    alarm_color = 0xE243;
  } else if (entity.state == "armed_away") {
    alarm_icon = icons::SHIELD_LOCK;
    alarm_color = 0xE243;
  } else if (entity.state == "armed_night") {
    alarm_icon = icons::SHIELD_MOON;
    alarm_color = 0xE243;
  } else if (entity.state == "armed_vacation") {
    alarm_icon = icons::SHIELD_AIRPLANE;
    alarm_color = 0xE243;
  } else if (entity.state == "armed_custom_bypass") {
    alarm_icon = icons::SHIELD;
    alarm_color = 0xE243;
  } else if (entity.state == "arming" || entity.state == "disarming" || entity.state == "pending") {
    alarm_icon = icons::SHIELD;
    alarm_color = 0xED80;
  } else if (entity.state == "triggered") {
    alarm_icon = icons::BELL_RING;
    alarm_color = 0xE243;
  }

  const bool flashing = entity.state == "triggered" || entity.state == "arming" || entity.state == "disarming" ||
                        entity.state == "pending";
  std::vector<std::string> arm_modes;
  uint16_t supported_features = 0;
  if (entity.attributes.count(ATTR_SUPPORTED_FEATURES)) {
    parse_int_(entity.attributes.at(ATTR_SUPPORTED_FEATURES), supported_features);
  }
  if (supported_features & 0b000001) arm_modes.push_back("arm_home");
  if (supported_features & 0b000010) arm_modes.push_back("arm_away");
  if (supported_features & 0b000100) arm_modes.push_back("arm_night");
  if (supported_features & 0b100000) arm_modes.push_back("arm_vacation");

  std::string command{"entityUpd~"};
  command.append(protocol_escape_(card.title)).append("~");
  this->render_card_navigation_(command);
  command.append("~").append(entity.entity_id);

  if (entity.state == "unknown" || entity.state == "disarmed") {
    size_t count = 0;
    for (const auto &mode : arm_modes) {
      if (count >= 4) break;
      command.append("~").append(this->get_translation_(mode)).append("~").append(mode);
      ++count;
    }
    command.append(2 * (4 - count), '~');
  } else {
    command.append("~").append(this->get_translation_("disarm")).append("~disarm");
    command.append(6, '~');
  }

  command.append("~").append(alarm_icon).append("~").append(std::to_string(alarm_color));

  command.append("~enable");

  command.append("~").append(flashing ? "enable" : "disable");

  const bool has_open_sensors = entity.attributes.count(ATTR_OPEN_SENSORS) && !entity.attributes.at(ATTR_OPEN_SENSORS).empty();
  if (has_open_sensors) {
    command.append("~").append(icons::ALERT_CIRCLE_OUTLINE).append("~").append(std::to_string(0xED80));
  }

  this->send_display_command(std::move(command));
}

}  // namespace nspanel_lovelace
}  // namespace esphome
