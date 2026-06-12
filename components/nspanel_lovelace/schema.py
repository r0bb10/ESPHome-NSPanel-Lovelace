import esphome.config_validation as cv

from .const import (
    CONF_ACTIVE,
    CONF_BRIGHTNESS,
    CONF_MODEL,
    CONF_SCREENSAVER,
    CONF_SLEEP_TIMEOUT,
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
