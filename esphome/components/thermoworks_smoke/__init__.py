import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import pins
from esphome.components import spi, sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["spi"]
AUTO_LOAD = ["sensor"]

CONF_RF_CHANNEL = "rf_channel"
CONF_RADIO_ID = "radio_id"
CONF_ALT_ID = "alt_radio_id"
CONF_CE_PIN = "ce_pin"
CONF_CS_PIN = "cs_pin"

CONF_PROBE1_TEMP = "probe1_temp_sensor"
CONF_PROBE1_MIN = "probe1_min_sensor"
CONF_PROBE1_MAX = "probe1_max_sensor"
CONF_PROBE2_TEMP = "probe2_temp_sensor"
CONF_PROBE2_MIN = "probe2_min_sensor"
CONF_PROBE2_MAX = "probe2_max_sensor"

thermoworks_smoke_ns = cg.esphome_ns.namespace("thermoworks_smoke")
SmokeReceiverComponent = thermoworks_smoke_ns.class_("SmokeReceiverComponent", cg.Component, spi.SPIDevice)

CONFIG_SCHEMA = cv.Schema(
{
    cv.GenerateID(): cv.declare_id(SmokeReceiverComponent),
    cv.Required(CONF_RF_CHANNEL): cv.uint8_t,
    cv.Optional(CONF_RADIO_ID): cv.uint64_t,
    cv.Optional(CONF_ALT_ID): cv.uint8_t,
    cv.Required(CONF_CE_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_CS_PIN): pins.gpio_input_pin_schema,
    
    cv.Optional(CONF_PROBE1_TEMP):
        sensor.sensor_schema(device_class="temperature",unit_of_measurement="°F",accuracy_decimals=1,state_class="measurement").extend(),
    cv.Optional(CONF_PROBE1_MIN):
        sensor.sensor_schema(device_class="temperature",unit_of_measurement="°F",accuracy_decimals=1,state_class="measurement").extend(),
    cv.Optional(CONF_PROBE1_MAX):
        sensor.sensor_schema(device_class="temperature",unit_of_measurement="°F",accuracy_decimals=1,state_class="measurement").extend(),
       
    cv.Optional(CONF_PROBE2_TEMP):
        sensor.sensor_schema(device_class="temperature",unit_of_measurement="°F",accuracy_decimals=1,state_class="measurement").extend(),
    cv.Optional(CONF_PROBE2_MIN):
        sensor.sensor_schema(device_class="temperature",unit_of_measurement="°F",accuracy_decimals=1,state_class="measurement").extend(),
    cv.Optional(CONF_PROBE2_MAX):
        sensor.sensor_schema(device_class="temperature",unit_of_measurement="°F",accuracy_decimals=1,state_class="measurement").extend(),       
        
}
).extend(cv.COMPONENT_SCHEMA).extend(cv.polling_component_schema('60s'))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    cg.add(var.set_rf_chan(config[CONF_RF_CHANNEL]))
    
    p = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    cg.add(var.set_cs_pin(p))
    p = await cg.gpio_pin_expression(config[CONF_CE_PIN])
    cg.add(var.set_ce_pin(p))
    
    if CONF_RADIO_ID in config:
        cg.add(var.set_radio_id(config[CONF_RADIO_ID]))
        
    if CONF_ALT_ID in config:
        cg.add(var.set_use_alt_id(config[CONF_ALT_ID]))
    
    if CONF_PROBE1_TEMP in config:
        conf = config[CONF_PROBE1_TEMP]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_probe1_temp_sensor(sens))
        
    if CONF_PROBE1_MIN in config:
        conf = config[CONF_PROBE1_MIN]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_probe1_min_sensor(sens))
        
    if CONF_PROBE1_MAX in config:
        conf = config[CONF_PROBE1_MAX]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_probe1_max_sensor(sens))

    if CONF_PROBE2_TEMP in config:
        conf = config[CONF_PROBE2_TEMP]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_probe2_temp_sensor(sens))
        
    if CONF_PROBE2_MIN in config:
        conf = config[CONF_PROBE2_MIN]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_probe2_min_sensor(sens))
        
    if CONF_PROBE2_MAX in config:
        conf = config[CONF_PROBE2_MAX]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_probe2_max_sensor(sens))
        
    
    cg.add_library("nrf24/RF24", None)
    cg.add_library("SPI", None)

# https://github.com/thegroove/esphome-custom-component-examples/blob/master/custom_components/empty_compound_sensor/sensor.py
# https://github.com/grob6000/esphome-externalcomponents/blob/master/watermeter.yaml
# https://github.com/wingrunr21/esphome-custom-components/blob/main/configuration.yaml
