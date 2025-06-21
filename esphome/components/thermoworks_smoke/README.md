esphome:
  name: esp-smoke
  friendly_name: esp-smoke

external_components:
  - source:
      type: local
      path: ./components
    components: [thermoworks_smoke]
  # source:
  #   type: git
  #   url: https://github.com/stefslon/esphome-thermoworks-smoke

esp8266:
  board: nodemcuv2

# Enable logging
logger:
  level: INFO

# Fill these out as needed
api:

ota:
  
wifi:

captive_portal:

spi:
  clk_pin: GPIO14
  mosi_pin: GPIO13
  miso_pin: GPIO12

# Change these pins as needed for your setup!
thermoworks_smoke:
  id: thermoworks_smoke_receiver
  rf_channel: 10
  radio_id: 0x123456789A # Need to be set to whatever your unit's ID is
  ce_pin: D4
  cs_pin: D1
  probe1_temp_sensor:
    name: "Smoke Probe 1 Temperature"
  probe1_min_sensor:
    name: "Smoke Probe 1 Min Alarm"
  probe1_max_sensor:
    name: "Smoke Probe 1 Max Alarm"
  probe2_temp_sensor:
    name: "Smoke Probe 2 Temperature"
  probe2_min_sensor:
    name: "Smoke Probe 2 Min Alarm"
  probe2_max_sensor:
    name: "Smoke Probe 2 Max Alarm"