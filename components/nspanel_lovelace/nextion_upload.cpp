#include "nspanel_lovelace.h"

#ifdef USE_NSPANEL_TFT_UPLOAD
#ifdef USE_ESP32

#include <esp_http_client.h>
#include <cinttypes>

#include "esphome/components/network/util.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace nspanel_lovelace {

static const char *const TAG = "nspanel_lovelace.upload";
static constexpr uint32_t UPLOAD_ACK_TIMEOUT = 5000;

// --- TFT upload ---

void NSPanelLovelace::upload_tft_service_(std::string url) {
  this->upload_tft(url);
}

void NSPanelLovelace::register_tft_upload_service() {
  this->register_service(&NSPanelLovelace::upload_tft_service_, "upload_tft", {"url"});
}

// --- TFT upload internals ---

uint16_t NSPanelLovelace::recv_ret_string_(std::string &response, uint32_t timeout) {
  const uint32_t deadline = millis() + timeout;
  response.clear();
  while (millis() < deadline) {
    uint8_t byte;
    while (this->available()) {
      this->read_byte(&byte);
      response.push_back(static_cast<char>(byte));
    }
    if (!response.empty()) {
      break;
    }
    delay(2);
    App.feed_wdt();
  }
  return response.size();
}

bool NSPanelLovelace::upload_tft(const std::string &url) {
  ESP_LOGI(TAG, "TFT upload requested");
  ESP_LOGD(TAG, "URL: %s", url.c_str());

  if (this->is_updating_) {
    ESP_LOGW(TAG, "Upload already in progress");
    return false;
  }

  if (!network::is_connected()) {
    ESP_LOGE(TAG, "Network not connected");
    return false;
  }

  this->is_updating_ = true;

  ESP_LOGD(TAG, "Waking display");
  this->transport_.send_raw_nextion_command("sleep=0");
  this->transport_.send_raw_nextion_command("dim=100");
  this->transport_.send_raw_nextion_command("dimmode~100~100");
  delay(250);

  esp_http_client_config_t config = {};
  config.url = url.c_str();
  config.method = HTTP_METHOD_HEAD;
  config.timeout_ms = 15000;
  config.disable_auto_redirect = false;
  config.max_redirection_count = 10;

  esp_http_client_handle_t http_client = esp_http_client_init(&config);
  if (http_client == nullptr) {
    ESP_LOGE(TAG, "HTTP client init failed");
    this->is_updating_ = false;
    return false;
  }

  esp_err_t err = esp_http_client_set_header(http_client, "Connection", "keep-alive");
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Set header failed: %s", esp_err_to_name(err));
    esp_http_client_cleanup(http_client);
    this->is_updating_ = false;
    return false;
  }

  err = esp_http_client_perform(http_client);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "HTTP HEAD failed: %s", esp_err_to_name(err));
    esp_http_client_cleanup(http_client);
    this->is_updating_ = false;
    return false;
  }

  int status_code = esp_http_client_get_status_code(http_client);
  if (status_code != 200 && status_code != 206) {
    ESP_LOGE(TAG, "HTTP status: %d", status_code);
    esp_http_client_cleanup(http_client);
    this->is_updating_ = false;
    return false;
  }

  this->tft_size_ = esp_http_client_get_content_length(http_client);
  ESP_LOGD(TAG, "TFT size: %zu bytes", this->tft_size_);

  if (this->tft_size_ < 4096 || this->tft_size_ > 134217728) {
    ESP_LOGE(TAG, "Invalid TFT size: %zu", this->tft_size_);
    esp_http_client_close(http_client);
    esp_http_client_cleanup(http_client);
    this->is_updating_ = false;
    return false;
  }

  this->content_length_ = this->tft_size_;

  char command[128];
  snprintf(command, sizeof(command), "whmi-wris %" PRIu32 ",%" PRIu32 ",1",
           this->content_length_, this->original_baud_rate_);

  uint8_t d;
  while (this->available()) {
    this->read_byte(&d);
  }

  this->transport_.send_raw_nextion_command(command);

  if (this->parent_->get_baud_rate() != this->original_baud_rate_) {
    this->parent_->set_baud_rate(this->original_baud_rate_);
    this->parent_->load_settings();
  }

  std::string response;
  this->recv_ret_string_(response, 5000);

  if (response.find(0x05) == std::string::npos) {
    ESP_LOGE(TAG, "Display rejected upload: 0x%02X",
             response.empty() ? 0 : static_cast<uint8_t>(response[0]));
    esp_http_client_close(http_client);
    esp_http_client_cleanup(http_client);
    return this->upload_end_(false);
  }

  ESP_LOGD(TAG, "Display ready for upload");
  esp_http_client_set_method(http_client, HTTP_METHOD_GET);
  ESP_LOGD(TAG, "Uploading TFT: %s (%zu bytes)", url.c_str(), this->tft_size_);

  uint32_t position = 0;
  while (this->content_length_ > 0) {
    // Determine this chunk's range
    uint32_t range_end = ((this->upload_first_chunk_sent_ || this->tft_size_ < 4096)
                           ? this->tft_size_ : 4096) - 1;
    if (range_end <= position) {
      ESP_LOGE(TAG, "Invalid range: %" PRIu32 " - %" PRIu32, position, range_end);
      esp_http_client_close(http_client);
      esp_http_client_cleanup(http_client);
      return this->upload_end_(false);
    }

    char range_header[32];
    buf_append_printf(range_header, sizeof(range_header), 0,
                      "bytes=%" PRIu32 "-%" PRIu32, position, range_end);
    ESP_LOGV(TAG, "Range: %s", range_header);
    esp_http_client_set_header(http_client, "Range", range_header);

    err = esp_http_client_open(http_client, 0);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
      esp_http_client_close(http_client);
      esp_http_client_cleanup(http_client);
      return this->upload_end_(false);
    }

    const int http_chunk_size = esp_http_client_fetch_headers(http_client);
    if (http_chunk_size <= 0) {
      ESP_LOGE(TAG, "HTTP fetch headers failed: %d", http_chunk_size);
      esp_http_client_close(http_client);
      esp_http_client_cleanup(http_client);
      return this->upload_end_(false);
    }

    ExternalRAMAllocator<uint8_t> allocator(ExternalRAMAllocator<uint8_t>::ALLOW_FAILURE);
    uint8_t *buffer = allocator.allocate(4096);
    if (buffer == nullptr) {
      ESP_LOGE(TAG, "Buffer alloc failed");
      esp_http_client_close(http_client);
      esp_http_client_cleanup(http_client);
      return this->upload_end_(false);
    }

    // Read and upload one 4KB segment
    App.feed_wdt();
    const uint16_t buf_size = this->content_length_ < 4096 ? this->content_length_ : 4096;
    uint16_t read_len = 0;
    int partial_read_len = 0;
    uint8_t retries = 0;

    while (retries < 5 && read_len < buf_size) {
      partial_read_len = esp_http_client_read(http_client,
          reinterpret_cast<char *>(buffer) + read_len, buf_size - read_len);
      if (partial_read_len > 0) {
        read_len += partial_read_len;
        retries = 0;
      } else {
        retries++;
        vTaskDelay(pdMS_TO_TICKS(2));
      }
      App.feed_wdt();
    }

    if (read_len != buf_size) {
      ESP_LOGE(TAG, "HTTP read failed: %" PRIu16 "/%" PRIu16 " bytes", read_len, buf_size);
      allocator.deallocate(buffer, 4096);
      esp_http_client_close(http_client);
      esp_http_client_cleanup(http_client);
      return this->upload_end_(false);
    }

    this->write_array(buffer, buf_size);
    App.feed_wdt();

    // Read display ack
    std::string ack;
    this->recv_ret_string_(ack, UPLOAD_ACK_TIMEOUT);

    if (!ack.empty() && ack[0] == 0x08 && ack.size() < 5) {
      const uint32_t deadline = millis() + UPLOAD_ACK_TIMEOUT;
      while (ack.size() < 5 && millis() < deadline) {
        if (this->available()) {
          uint8_t b;
          if (this->read_byte(&b)) {
            ack.push_back(static_cast<char>(b));
          }
        } else {
          vTaskDelay(pdMS_TO_TICKS(5));
          App.feed_wdt();
        }
      }
      if (ack.size() < 5) {
        ESP_LOGE(TAG, "Truncated 0x08 response: %zu bytes", ack.size());
        allocator.deallocate(buffer, 4096);
        esp_http_client_close(http_client);
        esp_http_client_cleanup(http_client);
        return this->upload_end_(false);
      }
    }

    this->content_length_ -= buf_size;
    ESP_LOGD(TAG, "Upload: %0.2f%% (%" PRIu32 " left)",
             100.0f * (this->tft_size_ - this->content_length_) / this->tft_size_,
             this->content_length_);
    this->upload_first_chunk_sent_ = true;

    if (ack.empty()) {
      ESP_LOGW(TAG, "No response from display after %" PRIu32 "ms", UPLOAD_ACK_TIMEOUT);
      allocator.deallocate(buffer, 4096);
      esp_http_client_close(http_client);
      esp_http_client_cleanup(http_client);
      return this->upload_end_(false);
    }

    if (ack[0] == 0x08 && ack.size() == 5) {
      uint32_t result = 0;
      for (int j = 0; j < 4; ++j) {
        result += static_cast<uint8_t>(ack[j + 1]) << (8 * j);
      }
      if (result > 0) {
        ESP_LOGI(TAG, "Nextion requested range restart at %" PRIu32, result);
        this->content_length_ = this->tft_size_ - result;
        position = result;
        esp_http_client_close(http_client);
        char new_range[32];
        buf_append_printf(new_range, sizeof(new_range), 0,
                          "bytes=%" PRIu32 "-%" PRIu32,
                          position, static_cast<uint32_t>(this->tft_size_));
        esp_http_client_set_header(http_client, "Range", new_range);
      } else {
        position = range_end + 1;
      }
      allocator.deallocate(buffer, 4096);
      continue;
    }

    if (ack[0] != 0x05) {
      ESP_LOGE(TAG, "Invalid response: 0x%02X", static_cast<uint8_t>(ack[0]));
      allocator.deallocate(buffer, 4096);
      esp_http_client_close(http_client);
      esp_http_client_cleanup(http_client);
      return this->upload_end_(false);
    }

    position = range_end + 1;
    allocator.deallocate(buffer, 4096);
  }

  ESP_LOGD(TAG, "TFT upload complete");
  esp_http_client_close(http_client);
  esp_http_client_cleanup(http_client);
  return this->upload_end_(true);
}

bool NSPanelLovelace::upload_end_(bool successful) {
  if (successful) {
    ESP_LOGI(TAG, "TFT upload successful, waiting for display reboot");
  } else {
    ESP_LOGE(TAG, "TFT upload failed");
    if (this->parent_->get_baud_rate() != this->original_baud_rate_) {
      this->parent_->set_baud_rate(this->original_baud_rate_);
      this->parent_->load_settings();
    }
  }

  this->is_updating_ = false;

  if (successful) {
    this->flush();
    delay(2000);
    uint8_t d;
    while (this->available()) {
      this->read_byte(&d);
    }
  }

  return successful;
}

}  // namespace nspanel_lovelace
}  // namespace esphome

#endif  // USE_ESP32
#endif  // USE_NSPANEL_TFT_UPLOAD
