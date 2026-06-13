#include "nspanel_lovelace.h"

#include "helpers.h"
#include "icons.h"

namespace esphome {
namespace nspanel_lovelace {

namespace {

const char *const ATTR_MEDIA_TITLE = "media_title";
const char *const ATTR_MEDIA_ARTIST = "media_artist";
const char *const ATTR_MEDIA_CONTENT_TYPE = "media_content_type";
const char *const ATTR_VOLUME_LEVEL = "volume_level";
const char *const ATTR_SHUFFLE = "shuffle";
const char *const ATTR_SUPPORTED_FEATURES = "supported_features";

const char *media_type_icon(const std::string &content_type) {
  if (content_type == "music") return icons::MUSIC_NOTE;
  if (content_type == "tv" || content_type == "movie") return icons::MOVIE;
  if (content_type == "video") return icons::VIDEO;
  return icons::MUSIC;
}

}  // namespace

void NSPanelLovelace::add_card_media(std::string title, std::string entity_id) {
  this->cards_.push_back(CardPage{"cardMedia", std::move(title), "", {CardEntity{std::move(entity_id), "", "", 17299, "", {}}}});
}

void NSPanelLovelace::render_card_media_(const CardPage &card) {
  if (card.entities.empty()) {
    return;
  }
  const auto &entity = card.entities[0];

  const auto media_title = entity.attributes.count(ATTR_MEDIA_TITLE) ? entity.attributes.at(ATTR_MEDIA_TITLE) : "";
  const auto media_artist = entity.attributes.count(ATTR_MEDIA_ARTIST) ? entity.attributes.at(ATTR_MEDIA_ARTIST) : "";
  const auto content_type = entity.attributes.count(ATTR_MEDIA_CONTENT_TYPE) ? entity.attributes.at(ATTR_MEDIA_CONTENT_TYPE) : "";

  float volume = 0.0f;
  if (entity.attributes.count(ATTR_VOLUME_LEVEL)) {
    parse_float_(entity.attributes.at(ATTR_VOLUME_LEVEL), volume);
  }
  auto volume_pct = std::to_string(static_cast<uint8_t>(volume * 100.0f));

  const char *play_icon = entity.state == "playing" ? icons::PAUSE : icons::PLAY;

  uint32_t supported_features = 0;
  if (entity.attributes.count(ATTR_SUPPORTED_FEATURES)) {
    parse_int_(entity.attributes.at(ATTR_SUPPORTED_FEATURES), supported_features);
  }

  std::string on_off_color = "disable";
  if (supported_features & 0b10000000) {
    on_off_color = entity.state == "off" ? "1374" : "64704";
  }

  std::string shuffle_icon = "disable";
  if (supported_features & 0b100000000000000) {
    if (entity.attributes.count(ATTR_SHUFFLE)) {
      shuffle_icon = entity.attributes.at(ATTR_SHUFFLE) == "on" ? icons::SHUFFLE : icons::SHUFFLE_DISABLE;
    } else {
      shuffle_icon = icons::SHUFFLE_DISABLE;
    }
  }

  std::string command{"entityUpd~"};
  command.append(protocol_escape_(card.title)).append("~");
  this->render_card_navigation_(command);
  command.append("~").append(entity.entity_id)
      .append("~").append(media_title.substr(0, 40)).append("~~")
      .append(media_artist.substr(0, 40)).append("~~")
      .append(volume_pct).append("~")
      .append(play_icon).append("~")
      .append(on_off_color).append("~")
      .append(shuffle_icon).append("~")
      .append("media_pl").append("~")
      .append(entity.entity_id).append("~")
      .append(media_type_icon(content_type)).append("~")
      .append("17299~~");

  // remaining entities (source speakers)
  for (size_t i = 1; i < card.entities.size(); ++i) {
    this->append_card_entity_(command, card.entities[i]);
  }
  if (card.entities.size() > 1) {
    command.append("~");
  }

  this->send_display_command(std::move(command));
}

}  // namespace nspanel_lovelace
}  // namespace esphome
