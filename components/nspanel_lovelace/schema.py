import esphome.config_validation as cv

from .const import CONF_MODEL, MODEL_EU, MODEL_OPTIONS


DISPLAY_SCHEMA = cv.Schema({
    cv.Optional(CONF_MODEL, default=MODEL_EU): cv.one_of(*MODEL_OPTIONS),
})
