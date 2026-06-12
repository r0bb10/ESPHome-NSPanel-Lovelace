import esphome.codegen as cg

from .const import (
    CONF_ACTIVE,
    CONF_BRIGHTNESS,
    CONF_DISPLAY,
    CONF_MODEL,
    CONF_SCREENSAVER,
    CONF_SLEEP_TIMEOUT,
)


async def build_component(var, config):
    display_config = config[CONF_DISPLAY]
    cg.add(var.set_model(display_config[CONF_MODEL]))
    cg.add(var.set_sleep_timeout(display_config[CONF_SLEEP_TIMEOUT]))

    brightness_config = display_config[CONF_BRIGHTNESS]
    cg.add(var.set_active_brightness(brightness_config[CONF_ACTIVE]))
    cg.add(var.set_screensaver_brightness(brightness_config[CONF_SCREENSAVER]))
