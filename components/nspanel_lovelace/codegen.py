import json
from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv

from .const import (
    CONF_ACTIVE,
    CONF_BRIGHTNESS,
    CONF_DISPLAY,
    CONF_LANGUAGE,
    CONF_LOCALE,
    CONF_MODEL,
    CONF_SCREENSAVER,
    CONF_WEATHER,
    CONF_SLEEP_TIMEOUT,
    CONF_TIME_FORMAT,
    CONF_TIME_ID,
    CONF_DATE_FORMAT,
    CONF_ENTITIES,
    CONF_ENTITY_ID,
    CONF_ICON,
    CONF_COLOR,
    CONF_NAME,
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
    locale_config = config[CONF_LOCALE]
    language = locale_config[CONF_LANGUAGE]
    cg.add(var.set_language(language))
    cg.add(var.set_time_format(locale_config[CONF_TIME_FORMAT]))
    cg.add(var.set_date_format(locale_config[CONF_DATE_FORMAT]))
    for key, value in load_translations(language).items():
        if key in TRANSLATION_KEYS:
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
                weather_config[CONF_ICON],
                weather_config[CONF_COLOR],
            ))
        for entity_config in screensaver_config[CONF_ENTITIES]:
            cg.add(var.add_screensaver_entity(
                entity_config[CONF_ENTITY_ID],
                entity_config.get(CONF_NAME, ""),
                entity_config[CONF_ICON],
                entity_config[CONF_COLOR],
            ))
