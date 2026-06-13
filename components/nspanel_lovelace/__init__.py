import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

from .codegen import build_component
from .const import CONF_CARDS, CONF_DISPLAY, CONF_LOCALE, CONF_SCREENSAVER, CONF_TFT_UPLOAD
from .schema import CARDS_SCHEMA, DISPLAY_SCHEMA, LOCALE_SCHEMA, SCREENSAVER_SCHEMA

CODEOWNERS = ["@r0bb10"]
DEPENDENCIES = ["api", "uart"]
AUTO_LOAD = ["json"]

nspanel_lovelace_ns = cg.esphome_ns.namespace("nspanel_lovelace")
NSPanelLovelace = nspanel_lovelace_ns.class_(
    "NSPanelLovelace", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(NSPanelLovelace),
    cv.Optional(CONF_LOCALE, default={}): LOCALE_SCHEMA,
    cv.Required(CONF_DISPLAY): DISPLAY_SCHEMA,
    cv.Optional(CONF_SCREENSAVER): SCREENSAVER_SCHEMA,
    cv.Optional(CONF_CARDS): CARDS_SCHEMA,
    cv.Optional(CONF_TFT_UPLOAD, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    await build_component(var, config)
