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
    CONF_TYPE,
    CONF_TITLE,
    CONF_ENTITIES,
    CONF_NAME,
    CONF_QR_TEXT,
    CONF_ALARM_SUPPORTED_MODES,
    CARD_ENTITIES,
    CARD_GRID,
    CARD_QR,
    CARD_THERMO,
    CARD_ALARM,
    CARD_MEDIA,
    ALARM_ARM_HOME,
    ALARM_ARM_AWAY,
    ALARM_ARM_OPTIONS,
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


SCREENSAVER_EXTRA_ENTITY_SCHEMA = cv.Schema({
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_ICON, default=""): cv.icon,
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


SCREENSAVER_STATUS_ICON_SCHEMA = cv.Schema({
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_ICON, default="mdi:alert-circle-outline"): cv.icon,
    cv.Optional(CONF_COLOR, default=65535): cv.int_range(0, 65535),
    cv.Optional(CONF_ALT_FONT, default=False): cv.boolean,
})


SCREENSAVER_SCHEMA = cv.Schema({
    cv.Required(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    cv.Optional(CONF_WEATHER): SCREENSAVER_WEATHER_SCHEMA,
    cv.Optional(CONF_FORECAST): SCREENSAVER_FORECAST_SCHEMA,
    cv.Optional(CONF_EXTRA_ENTITY): SCREENSAVER_EXTRA_ENTITY_SCHEMA,
    cv.Optional(CONF_STATUS_ICON_LEFT): SCREENSAVER_STATUS_ICON_SCHEMA,
    cv.Optional(CONF_STATUS_ICON_RIGHT): SCREENSAVER_STATUS_ICON_SCHEMA,
})


CARD_ENTITY_SCHEMA = cv.Schema({
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_NAME, default=""): cv.string_strict,
    cv.Optional(CONF_ICON, default=""): cv.icon,
    cv.Optional(CONF_COLOR, default=17299): cv.int_range(0, 65535),
})


CARD_WITH_ENTITIES_SCHEMA = cv.Schema({
    cv.Required(CONF_TYPE): cv.one_of(CARD_ENTITIES, CARD_GRID),
    cv.Optional(CONF_TITLE, default=""): cv.string_strict,
    cv.Required(CONF_ENTITIES): cv.All(cv.ensure_list(CARD_ENTITY_SCHEMA), cv.Length(min=1, max=6)),
})


CARD_QR_SCHEMA = cv.Schema({
    cv.Required(CONF_TYPE): cv.one_of(CARD_QR),
    cv.Optional(CONF_TITLE, default=""): cv.string_strict,
    cv.Required(CONF_QR_TEXT): cv.string_strict,
    cv.Optional(CONF_ENTITIES, default=[]): cv.All(cv.ensure_list(CARD_ENTITY_SCHEMA), cv.Length(max=2)),
})


CARD_THERMO_SCHEMA = cv.Schema({
    cv.Required(CONF_TYPE): cv.one_of(CARD_THERMO),
    cv.Optional(CONF_TITLE, default=""): cv.string_strict,
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
})


CARD_ALARM_SCHEMA = cv.Schema({
    cv.Required(CONF_TYPE): cv.one_of(CARD_ALARM),
    cv.Optional(CONF_TITLE, default=""): cv.string_strict,
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_ALARM_SUPPORTED_MODES, default=[ALARM_ARM_HOME, ALARM_ARM_AWAY]): cv.All(
        cv.ensure_list(cv.one_of(*ALARM_ARM_OPTIONS)),
        cv.Length(min=1, max=4),
    ),
})


CARD_MEDIA_SCHEMA = cv.Schema({
    cv.Required(CONF_TYPE): cv.one_of(CARD_MEDIA),
    cv.Optional(CONF_TITLE, default=""): cv.string_strict,
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_ENTITIES, default=[]): cv.All(cv.ensure_list(CARD_ENTITY_SCHEMA), cv.Length(max=6)),
})


CARDS_SCHEMA = cv.All(
    cv.ensure_list(cv.Any(CARD_WITH_ENTITIES_SCHEMA, CARD_QR_SCHEMA, CARD_THERMO_SCHEMA, CARD_ALARM_SCHEMA, CARD_MEDIA_SCHEMA)),
    cv.Length(min=1),
)
