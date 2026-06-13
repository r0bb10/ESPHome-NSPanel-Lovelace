import esphome.config_validation as cv
from esphome.components import time

from .const import (
    CONF_ACTIVE,
    CONF_BRIGHTNESS,
    CONF_LANGUAGE,
    CONF_LOCALE,
    CONF_MODEL,
    CONF_SCREENSAVER,
    CONF_WEATHER,
    CONF_FORECAST,
    CONF_SLEEP_TIMEOUT,
    CONF_TIME_FORMAT,
    CONF_TIME_ID,
    CONF_DATE_FORMAT,
    CONF_ENTITIES,
    CONF_ENTITY_ID,
    CONF_ICON,
    CONF_COLOR,
    CONF_NAME,
    MODEL_EU,
    MODEL_OPTIONS,
)


TIME_FORMAT_PRESETS = {
    "24h": "%H:%M",
    "12h": "%I:%M %p",
}

DATE_FORMAT_PRESETS = {
    "long": "%A, %d. %B %Y",
    "short": "%d.%m.%Y",
    "compact": "%d.%m.",
    "iso": "%Y-%m-%d",
}


def format_preset(presets):
    def validator(value):
        value = cv.string_strict(value)
        return presets.get(value, value)

    return validator


BRIGHTNESS_SCHEMA = cv.Schema({
    cv.Optional(CONF_ACTIVE, default=100): cv.int_range(1, 100),
    cv.Optional(CONF_SCREENSAVER, default=20): cv.int_range(0, 100),
})


DISPLAY_SCHEMA = cv.Schema({
    cv.Optional(CONF_MODEL, default=MODEL_EU): cv.one_of(*MODEL_OPTIONS),
    cv.Optional(CONF_SLEEP_TIMEOUT, default=20): cv.int_range(0, 3600),
    cv.Optional(CONF_BRIGHTNESS, default={}): BRIGHTNESS_SCHEMA,
})


LOCALE_SCHEMA = cv.Schema({
    cv.Optional(CONF_LANGUAGE, default="en"): cv.string_strict,
    cv.Optional(CONF_TIME_FORMAT, default="24h"): format_preset(TIME_FORMAT_PRESETS),
    cv.Optional(CONF_DATE_FORMAT, default="long"): format_preset(DATE_FORMAT_PRESETS),
})


SCREENSAVER_ENTITY_SCHEMA = cv.Schema({
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_NAME): cv.string_strict,
    cv.Optional(CONF_ICON, default=""): cv.string_strict,
    cv.Optional(CONF_COLOR, default=65535): cv.int_range(0, 65535),
})


SCREENSAVER_WEATHER_SCHEMA = cv.Schema({
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_COLOR): cv.int_range(0, 65535),
})


SCREENSAVER_FORECAST_SCHEMA = cv.Schema({
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_COLOR): cv.int_range(0, 65535),
})


SCREENSAVER_SCHEMA = cv.Schema({
    cv.Required(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    cv.Optional(CONF_WEATHER): SCREENSAVER_WEATHER_SCHEMA,
    cv.Optional(CONF_FORECAST): SCREENSAVER_FORECAST_SCHEMA,
    cv.Optional(CONF_ENTITIES, default=[]): cv.ensure_list(SCREENSAVER_ENTITY_SCHEMA),
})
