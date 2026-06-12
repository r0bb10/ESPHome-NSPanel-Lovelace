import esphome.config_validation as cv
from esphome.components import time

from .const import (
    CONF_ACTIVE,
    CONF_BRIGHTNESS,
    CONF_MODEL,
    CONF_SCREENSAVER,
    CONF_SLEEP_TIMEOUT,
    CONF_TIME_FORMAT,
    CONF_TIME_ID,
    CONF_DATE_FORMAT,
    CONF_ENTITIES,
    CONF_ENTITY_ID,
    CONF_NAME,
    MODEL_EU,
    MODEL_OPTIONS,
)


BRIGHTNESS_SCHEMA = cv.Schema({
    cv.Optional(CONF_ACTIVE, default=100): cv.int_range(1, 100),
    cv.Optional(CONF_SCREENSAVER, default=20): cv.int_range(0, 100),
})


DISPLAY_SCHEMA = cv.Schema({
    cv.Optional(CONF_MODEL, default=MODEL_EU): cv.one_of(*MODEL_OPTIONS),
    cv.Optional(CONF_SLEEP_TIMEOUT, default=20): cv.int_range(0, 3600),
    cv.Optional(CONF_BRIGHTNESS, default={}): BRIGHTNESS_SCHEMA,
})


SCREENSAVER_ENTITY_SCHEMA = cv.Schema({
    cv.Required(CONF_ENTITY_ID): cv.string_strict,
    cv.Optional(CONF_NAME): cv.string_strict,
})


SCREENSAVER_SCHEMA = cv.Schema({
    cv.Required(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    cv.Optional(CONF_TIME_FORMAT, default="%H:%M"): cv.string_strict,
    cv.Optional(CONF_DATE_FORMAT, default="%A, %d. %B %Y"): cv.string_strict,
    cv.Optional(CONF_ENTITIES, default=[]): cv.ensure_list(SCREENSAVER_ENTITY_SCHEMA),
})
