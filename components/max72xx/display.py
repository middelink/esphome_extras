# coding=utf-8
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, spi
from esphome.const import CONF_ID, CONF_INTENSITY, CONF_LAMBDA, CONF_NUM_CHIPS, CONF_PAGES

DEPENDENCIES = ['display', 'spi']

max72xx_ns = cg.esphome_ns.namespace('max72xx')
MAX72XXComponent = max72xx_ns.class_('MAX72XXComponent', cg.PollingComponent, spi.SPIDevice)

CHAIN_DIRECTIONS = {
  'UP': max72xx_ns.CHAIN_DIRECTION_UP,
  'DOWN': max72xx_ns.CHAIN_DIRECTION_DOWN,
  'LEFT': max72xx_ns.CHAIN_DIRECTION_LEFT,
  'RIGHT': max72xx_ns.CHAIN_DIRECTION_RIGHT,
}
CONF_CHAIN_DIRECTION = 'chain_direction'
CONF_HW_DIG_ROWS = 'row_column_swapped'
CONF_HW_REV_ROWS = 'reverse_rows'
CONF_HW_REV_COLS = 'reverse_columns'


CONFIG_SCHEMA = cv.All(display.FULL_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(MAX72XXComponent),

    cv.Optional(CONF_CHAIN_DIRECTION, default='left'): cv.enum(CHAIN_DIRECTIONS, upper=True),
    cv.Optional(CONF_HW_DIG_ROWS, default=False): cv.boolean,
    cv.Optional(CONF_HW_REV_ROWS, default=False): cv.boolean,
    cv.Optional(CONF_HW_REV_COLS, default=False): cv.boolean,
    cv.Optional(CONF_NUM_CHIPS, default=1): cv.int_range(min=1, max=255),
    cv.Optional(CONF_INTENSITY): cv.int_range(min=0, max=15),
}).extend(cv.polling_component_schema('1s')).extend(spi.spi_device_schema()),
		cv.has_at_most_one_key(CONF_PAGES, CONF_LAMBDA))


def to_code(config):
    # Give the hardware related settings to the constructor, as changing them runtime will wreak havoc
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_NUM_CHIPS], config[CONF_CHAIN_DIRECTION], config[CONF_HW_DIG_ROWS], config[CONF_HW_REV_ROWS], config[CONF_HW_REV_COLS])
    yield cg.register_component(var, config)
    yield spi.register_spi_device(var, config)
    yield display.register_display(var, config)

    if CONF_INTENSITY in config:
      cg.add(var.set_intensity(config[CONF_INTENSITY]))

    if CONF_LAMBDA in config:
      lambda_ = yield cg.process_lambda(
		      config[CONF_LAMBDA], [(MAX72XXComponent.operator('ref'), 'it')], return_type=cg.void)
      cg.add(var.set_writer(lambda_))
