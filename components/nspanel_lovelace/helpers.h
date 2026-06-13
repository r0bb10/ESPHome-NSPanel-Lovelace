#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace esphome {
namespace nspanel_lovelace {

inline double scale_value(double value, std::array<double, 2> from, std::array<double, 2> to) {
  return ((value - from[0]) / (from[1] - from[0])) * (to[1] - to[0]) + to[0];
}

inline bool contains_value(const std::string &str, const char *value) {
  return str.find(value) != std::string::npos;
}

inline bool contains_value(const std::vector<std::string> &vec, const char *value) {
  for (const auto &item : vec) {
    if (item == value) return true;
  }
  return false;
}

inline std::vector<uint8_t> hsv2rgb(double h, double s, double v) {
  if (s <= 0.0) {
    auto val = static_cast<uint8_t>(round(v * 255));
    return {val, val, val};
  }

  auto i = static_cast<uint32_t>(h * 6.0);
  double f = (h * 6.0) - i;
  double p = v * (1.0 - s);
  double q = v * (1.0 - (s * f));
  double t = v * (1.0 - (s * (1.0 - f)));

  double r = 0, g = 0, b = 0;
  switch (i) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
  }

  return {
      static_cast<uint8_t>(round(r * 255)),
      static_cast<uint8_t>(round(g * 255)),
      static_cast<uint8_t>(round(b * 255))};
}

inline std::vector<uint8_t> xy_to_rgb(double x, double y, float wh) {
  double r = wh / 2;
  x = round((x - r) / r * 100) / 100;
  y = round((r - y) / r * 100) / 100;

  r = sqrt((x * x) + (y * y));
  return hsv2rgb(
      std::fmod((atan2(y, x) * (180 / M_PI)), 360) / 360,
      (r > 1 ? 0 : r),
      1);
}

}  // namespace nspanel_lovelace
}  // namespace esphome
