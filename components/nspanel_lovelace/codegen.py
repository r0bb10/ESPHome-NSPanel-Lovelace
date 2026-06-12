import esphome.codegen as cg

from .const import CONF_DISPLAY, CONF_MODEL


async def build_component(var, config):
    display_config = config[CONF_DISPLAY]
    cg.add(var.set_model(display_config[CONF_MODEL]))
