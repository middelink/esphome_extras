import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID, CONF_RESOLUTION, UNIT_LUX, ICON_BRIGHTNESS_5, DEVICE_CLASS_ILLUMINANCE, STATE_CLASS_MEASUREMENT

DEPENDENCIES = ['i2c']

apds9301_ns = cg.esphome_ns.namespace('apds9301')
APDS9301Resolution = apds9301_ns.enum('APDS9301Resolution')
APDS9301_RESOLUTIONS = {
    1.000*16: APDS9301Resolution.APDS9301_RESOLUTION_16P00_LX, # High gain
    0.252*16: APDS9301Resolution.APDS9301_RESOLUTION_4P024_LX,
    0.034*16: APDS9301Resolution.APDS9301_RESOLUTION_0P546_LX,
    1.000* 1: APDS9301Resolution.APDS9301_RESOLUTION_1P000_LX, # Low gain
    0.252* 1: APDS9301Resolution.APDS9301_RESOLUTION_0P252_LX,
    0.034* 1: APDS9301Resolution.APDS9301_RESOLUTION_0P034_LX,
}

APDS9301Sensor = apds9301_ns.class_('APDS9301Sensor', sensor.Sensor, cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        APDS9301Sensor,
        unit_of_measurement=UNIT_LUX,
        icon=ICON_BRIGHTNESS_5,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_ILLUMINANCE,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend({
        #cv.GenerateID(): cv.declare_id(APDS9301Sensor),
        cv.Optional(CONF_RESOLUTION, default=1.0): cv.enum(APDS9301_RESOLUTIONS, float=True),
    })
    .extend(cv.polling_component_schema('60s'))
    .extend(i2c.i2c_device_schema(0x39))
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    yield i2c.register_i2c_device(var, config)

    cg.add(var.set_resolution(config[CONF_RESOLUTION]))
