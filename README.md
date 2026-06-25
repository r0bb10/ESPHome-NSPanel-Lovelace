# ESPHome NSPanel Lovelace

This is a native ESPHome component for the Sonoff NSPanel using the Lovelace-style TFT/HMI from [joBr99/nspanel-lovelace-ui](https://github.com/joBr99/nspanel-lovelace-ui). This project is a rewrite of the excellent work done by [olicooper/esphome-nspanel-lovelace-native](https://github.com/olicooper/esphome-nspanel-lovelace-native), and that is why it's a fork of it even if the whole codebase was redone.

The idea is to have parity with joBr99's screen design and protocol, but run the backend directly on ESPHome as light as possible. Since i've done it for my personal use case it's proven to work on screensaver page (with the original alternative layout by enabling the 6th extra entity), weather, alarm and thermo and qr only.

The display needs the NSPanel Lovelace UI TFT/HMI exactly as per joBr99's documentation.

## Pre Requisite

Create a forecast sensor because the panel expects a sensor with a `forecast` attribute:

```yaml
template:
  - trigger:
      - platform: time_pattern
        minutes: /30
      - platform: homeassistant
        event: start
    action:
      - action: weather.get_forecasts
        target:
          entity_id: weather.openweathermap
        data:
          type: daily
        response_variable: forecast_data
    sensor:
      - name: OpenWeather Forecast Daily
        unique_id: openweather_forecast_daily
        state: "{{ states('weather.openweathermap') }}"
        attributes:
          forecast: >-
            {{ forecast_data['weather.openweathermap'].forecast[:5] }}
```

Use the resulting `sensor.openweather_forecast_daily` in `screensaver.forecast.entity_id`.

## Installation

Add this repository as an ESPHome external component:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/r0bb10/ESPHome-NSPanel-Lovelace
      ref: main
    refresh: always
    components: [nspanel_lovelace]
```

In Home Assistant, in the ESPHome device settings allow the device to perform Home Assistant actions.

## Minimal Example

```yaml

time:
  - platform: homeassistant
    id: homeassistant_time

uart:
  id: uart_nextion
  tx_pin: GPIO16
  rx_pin: GPIO17
  baud_rate: 115200

nspanel_lovelace:
  id: nspanel
  uart_id: uart_nextion

  display:
    model: eu
    sleep_timeout: 20
    brightness:
      active: 100
      screensaver: 20

  locale:
    language: en
    time_format: 24h
    date_format: long

  screensaver:
    time_id: homeassistant_time
    weather:
      entity_id: weather.openweathermap
      color: [255, 255, 255]
    forecast:
      entity_id: sensor.openweather_forecast_daily
      color: [255, 255, 255]
    extra_entity:
      entity_id: sensor.room_temperature
      icon: mdi:home-thermometer-outline
      color: [255, 255, 255]

  cards:
    - type: cardThermo
      title: Climate
      entity_id: climate.room
    - type: cardAlarm
      title: Alarm
      entity_id: alarm_control_panel.home
    - type: cardQR
      title: Guest WiFi
      ssid: !secret guest_ssid
      password: !secret guest_password
      auth: WPA
      entities:
        - name: SSID
          icon: mdi:wifi
          color: [255, 255, 255]
          value: !secret guest_ssid
        - name: Bands
          icon: mdi:wifi-settings
          color: [255, 255, 255]
          value: 2.4G / 5G
```

## TFT Upload

Set `tft_upload: true` to expose a Home Assistant service for TFT upload:

```yaml
nspanel_lovelace:
  id: nspanel
  uart_id: uart_nextion
  tft_upload: true
```

## Credits

This project depends on the work done by the NSPanel community:

- [joBr99/nspanel-lovelace-ui](https://github.com/joBr99/nspanel-lovelace-ui) for the Lovelace-style TFT/HMI, protocol, layouts, cards, and the original Home Assistant experience.
- [olicooper/esphome-nspanel-lovelace-native](https://github.com/olicooper/esphome-nspanel-lovelace-native) for the ESPHome-native direction and early component work.
