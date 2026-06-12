import esphome.codegen as cg

from .const import (
    CONF_ACTIVE,
    CONF_BRIGHTNESS,
    CONF_DISPLAY,
    CONF_MODEL,
    CONF_SCREENSAVER,
    CONF_SLEEP_TIMEOUT,
    CONF_TIME_FORMAT,
    CONF_TIME_ID,
    CONF_DATE_FORMAT,
)


async def build_component(var, config):
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
        cg.add(var.set_time_format(screensaver_config[CONF_TIME_FORMAT]))
        cg.add(var.set_date_format(screensaver_config[CONF_DATE_FORMAT]))
