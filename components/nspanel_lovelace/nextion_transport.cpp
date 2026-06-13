#include "nextion_transport.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace.transport";

void NextionTransport::send_command(const std::string &command) {
  if (this->uart_ == nullptr) {
    return;
  }

  const uint8_t header[4] = {
      0x55,
      0xBB,
      static_cast<uint8_t>(command.size() & 0xFF),
      static_cast<uint8_t>((command.size() >> 8) & 0xFF),
  };
  uint16_t crc = esphome::crc16(header, sizeof(header));
  crc = esphome::crc16(reinterpret_cast<const uint8_t *>(command.c_str()), command.size(), crc);

  this->uart_->write_array(header, sizeof(header));
  this->uart_->write_str(command.c_str());
  this->uart_->write_byte(static_cast<uint8_t>(crc & 0xFF));
  this->uart_->write_byte(static_cast<uint8_t>((crc >> 8) & 0xFF));
}

void NextionTransport::send_raw_nextion_command(const std::string &command) {
  if (this->uart_ == nullptr) {
    return;
  }

  this->uart_->write_str(command.c_str());
  this->uart_->write_byte(0xFF);
  this->uart_->write_byte(0xFF);
  this->uart_->write_byte(0xFF);
}

bool NextionTransport::read_payload(std::string &payload) {
  if (this->uart_ == nullptr) {
    return false;
  }

  uint8_t byte{0};
  while (this->uart_->available() && this->uart_->read_byte(&byte)) {
    this->rx_buffer_.push_back(byte);
  }

  while (!this->rx_buffer_.empty() && this->rx_buffer_[0] != 0x55) {
    this->rx_buffer_.erase(this->rx_buffer_.begin());
  }
  if (this->rx_buffer_.size() < 6) {
    return false;
  }
  if (this->rx_buffer_[1] != 0xBB) {
    this->rx_buffer_.erase(this->rx_buffer_.begin());
    return false;
  }

  const uint16_t length = static_cast<uint16_t>(this->rx_buffer_[2]) |
                          (static_cast<uint16_t>(this->rx_buffer_[3]) << 8);
  const size_t frame_length = 4u + length + 2u;
  if (length > 1024) {
    ESP_LOGW(TAG, "Dropping oversized display frame: %u", length);
    this->rx_buffer_.clear();
    return false;
  }
  if (this->rx_buffer_.size() < frame_length) {
    return false;
  }

  const uint16_t expected_crc = static_cast<uint16_t>(this->rx_buffer_[4 + length]) |
                                (static_cast<uint16_t>(this->rx_buffer_[5 + length]) << 8);
  const uint16_t actual_crc = esphome::crc16(this->rx_buffer_.data(), 4u + length);
  if (expected_crc != actual_crc) {
    ESP_LOGW(TAG, "Dropping display frame with invalid CRC: %04X != %04X", expected_crc, actual_crc);
    this->rx_buffer_.erase(this->rx_buffer_.begin());
    return false;
  }

  payload.assign(reinterpret_cast<const char *>(this->rx_buffer_.data() + 4), length);
  this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + frame_length);
  return true;
}

}  // namespace nspanel_lovelace
}  // namespace esphome
