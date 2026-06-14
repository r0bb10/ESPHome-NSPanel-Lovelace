import json
from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32
from esphome.core import CORE

from .const import (
    CONF_ACTIVE,
    CONF_BRIGHTNESS,
    CONF_DISPLAY,
    CONF_LANGUAGE,
    CONF_LOCALE,
    CONF_MODEL,
    CONF_SCREENSAVER,
    CONF_WEATHER,
    CONF_FORECAST,
    CONF_EXTRA_ENTITY,
    CONF_STATUS_ICON_LEFT,
    CONF_STATUS_ICON_RIGHT,
    CONF_ALT_FONT,
    CONF_SLEEP_TIMEOUT,
    CONF_TIME_FORMAT,
    CONF_TIME_ID,
    CONF_DATE_FORMAT,
    CONF_ENTITY_ID,
    CONF_ICON,
    CONF_COLOR,
    CONF_CARDS,
    CONF_TITLE,
    CONF_ENTITIES,
    CONF_NAME,
    CONF_VALUE,
    CONF_SSID,
    CONF_PASSWORD,
    CONF_AUTH,
    CONF_TYPE,
    CONF_QR_TEXT,
    CONF_TFT_UPLOAD,
    CARD_QR,
    CARD_THERMO,
    CARD_ALARM,
    CARD_MEDIA,
)


TRANSLATION_KEYS = (
    "month_january",
    "month_jan",
    "month_february",
    "month_feb",
    "month_march",
    "month_mar",
    "month_april",
    "month_apr",
    "month_may",
    "month_june",
    "month_jun",
    "month_july",
    "month_jul",
    "month_august",
    "month_aug",
    "month_september",
    "month_sep",
    "month_october",
    "month_oct",
    "month_november",
    "month_nov",
    "month_december",
    "month_dec",
    "dow_sunday",
    "dow_sun",
    "dow_monday",
    "dow_mon",
    "dow_tuesday",
    "dow_tue",
    "dow_wednesday",
    "dow_wed",
    "dow_thursday",
    "dow_thu",
    "dow_friday",
    "dow_fri",
    "dow_saturday",
    "dow_sat",
)


def load_translations(language):
    if language.endswith(".json"):
        path = Path(language)
    else:
        path = Path(__file__).with_name("translations") / f"{language}.json"

    try:
        with path.open(encoding="utf-8") as file:
            translations = json.load(file)
    except OSError as err:
        raise cv.Invalid(f"Failed to load translation file '{path}'") from err

    missing = [key for key in TRANSLATION_KEYS if key not in translations]
    if missing:
        raise cv.Invalid(f"Translation file '{path}' is missing keys: {missing}")

    return translations


async def build_component(var, config):
    # The component requires Home Assistant state subscriptions and service calls.
    cg.add_define("USE_API_HOMEASSISTANT_STATES")
    cg.add_define("USE_API_HOMEASSISTANT_SERVICES")

    locale_config = config[CONF_LOCALE]
    language = locale_config[CONF_LANGUAGE]
    cg.add(var.set_language(language))
    cg.add(var.set_time_format(locale_config[CONF_TIME_FORMAT]))
    cg.add(var.set_date_format(locale_config[CONF_DATE_FORMAT]))
    for key, value in load_translations(language).items():
        cg.add(var.set_translation(key, value))

    display_config = config[CONF_DISPLAY]
    cg.add(var.set_model(display_config[CONF_MODEL]))
    cg.add(var.set_sleep_timeout(display_config[CONF_SLEEP_TIMEOUT]))

    brightness_config = display_config[CONF_BRIGHTNESS]
    cg.add(var.set_active_brightness(brightness_config[CONF_ACTIVE]))
    cg.add(var.set_screensaver_brightness(brightness_config[CONF_SCREENSAVER]))

    if CONF_SCREENSAVER in config:
        screensaver_config = config[CONF_SCREENSAVER]
        cg.add(var.set_screensaver_enabled(True))
        time_var = await cg.get_variable(screensaver_config[CONF_TIME_ID])
        cg.add(var.set_time(time_var))
        if CONF_WEATHER in screensaver_config:
            weather_config = screensaver_config[CONF_WEATHER]
            cg.add(var.set_screensaver_weather(
                weather_config[CONF_ENTITY_ID],
                weather_config.get(CONF_COLOR, -1),
            ))
        if CONF_FORECAST in screensaver_config:
            forecast_config = screensaver_config[CONF_FORECAST]
            cg.add(var.set_screensaver_forecast(
                forecast_config[CONF_ENTITY_ID],
                forecast_config.get(CONF_COLOR, -1),
            ))
        if CONF_EXTRA_ENTITY in screensaver_config:
            entity_config = screensaver_config[CONF_EXTRA_ENTITY]
            cg.add(var.set_screensaver_extra_entity(
                entity_config[CONF_ENTITY_ID],
                entity_config[CONF_ICON],
                entity_config[CONF_COLOR],
            ))
        if CONF_STATUS_ICON_LEFT in screensaver_config:
            icon_config = screensaver_config[CONF_STATUS_ICON_LEFT]
            cg.add(var.set_screensaver_status_icon_left(
                icon_config[CONF_ENTITY_ID],
                icon_config[CONF_ICON],
                icon_config[CONF_COLOR],
                icon_config[CONF_ALT_FONT],
            ))
        if CONF_STATUS_ICON_RIGHT in screensaver_config:
            icon_config = screensaver_config[CONF_STATUS_ICON_RIGHT]
            cg.add(var.set_screensaver_status_icon_right(
                icon_config[CONF_ENTITY_ID],
                icon_config[CONF_ICON],
                icon_config[CONF_COLOR],
                icon_config[CONF_ALT_FONT],
            ))

    if config.get(CONF_TFT_UPLOAD):
        cg.add_define("USE_NSPANEL_TFT_UPLOAD")
        cg.add_define("USE_API_USER_DEFINED_ACTIONS")
        cg.add_define("USE_API_CUSTOM_SERVICES")
        esp32.include_builtin_idf_component("esp_http_client")
        cg.add(var.register_tft_upload_service())
        api_config = CORE.config.get("api")
        if api_config is not None:
            api_config["custom_services"] = True

    for card_config in config.get(CONF_CARDS, []):
        if card_config[CONF_TYPE] == CARD_QR:
            cg.add(var.add_card_qr(
                card_config[CONF_TITLE],
                card_config.get(CONF_QR_TEXT, ""),
                card_config.get(CONF_SSID, ""),
                card_config.get(CONF_PASSWORD, ""),
                card_config[CONF_AUTH],
            ))
            for row_config in card_config.get(CONF_ENTITIES, []):
                cg.add(var.add_card_entity(
                    "",
                    row_config[CONF_NAME],
                    row_config[CONF_ICON],
                    row_config[CONF_COLOR],
                    row_config[CONF_VALUE],
                ))
        elif card_config[CONF_TYPE] == CARD_THERMO:
            cg.add(var.add_card_thermo(
                card_config[CONF_TITLE],
                card_config[CONF_ENTITY_ID],
            ))
        elif card_config[CONF_TYPE] == CARD_ALARM:
            cg.add(var.add_card_alarm(
                card_config[CONF_TITLE],
                card_config[CONF_ENTITY_ID],
            ))
        elif card_config[CONF_TYPE] == CARD_MEDIA:
            cg.add(var.add_card_media(
                card_config[CONF_TITLE],
                card_config[CONF_ENTITY_ID],
            ))
        else:
            cg.add(var.add_card_entities(card_config[CONF_TYPE], card_config[CONF_TITLE]))
        if card_config[CONF_TYPE] != CARD_QR:
            for entity_config in card_config.get(CONF_ENTITIES, []):
                cg.add(var.add_card_entity(
                    entity_config[CONF_ENTITY_ID],
                    entity_config[CONF_NAME],
                    entity_config[CONF_ICON],
                    entity_config[CONF_COLOR],
                ))
