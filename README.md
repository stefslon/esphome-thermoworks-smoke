## Introduction

These files provide ESPHome to Home Assitant integration of Thermoworks' [Smoke® Remote BBQ Alarm Thermometer](https://www.thermoworks.com/smoke/). I use this integration to get Smoke temperature data into Home Assistant dashboard without Smoke® Gateway (Wi-Fi bridge).

### You will need:
- nRF24L01 Transceiver Module
- ESPHome capable microcontroller (e.g. ESP-12F D1, ESP8266, or ESP32)

### Steps:
1. Wire up nRF24L01 to microcontroller (there are plenty of tutorials out there, [here](https://github.com/stefslon/esphome-thermoworks-smoke/blob/main/imgs/Wiring.png) is one example take from this [website](https://projecthub.arduino.cc/tmekinyan/how-to-use-the-nrf24l01-module-with-arduino-813957))
2. Create new ESPHome integration using provided [esp-smoke.yaml](esp-smoke.yaml) as a template 
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/stefslon/esphome-thermoworks-smoke
    components: [thermoworks_smoke]

thermoworks_smoke:
  id: thermoworks_smoke_receiver
  rf_channel: 10
  # -- when radio_id is not defined, unit goes into search mode
  #    watch ESPHome log for potential matches. Do NOT put Smoke unit
  #    into Connection/Pairing mode. Search may take up to 5 minutes.
  #radio_id: 0x123456789A 
  # -- if you are not able to find the unit, try uncommenting the line
  #    below, your unit might have a different address pattern
  #alt_radio_id: 1
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
```
3. If you already know your Smoke's `radio_id` then great, you can skip the next step 
4. To find your Smoke's `radio_id` keep that field commented out, which will put your ESP device into search mode. Make sure Smoke is powered on and is NOT in pairing mode. Keep an eye on ESP's logs. In less than 5 minutes you should see an ID that was found. Verify that found temperatures match what you see on your device.
![ID Found](https://github.com/stefslon/esphome-thermoworks-smoke/blob/main/imgs/Found.png)
    - If ID is not found in five minutes try uncommenting `alt_radio_id` field and re-running the process again
5. Set `radio_id` field to your Smoke's ID and you are all set!
6. Optionally setup dashboard in Home Assitant (I used [ApexCharts Card](https://github.com/RomRider/apexcharts-card) for graphs)

## ApexCharts Example
Below is an example of ApexCharts Card configuration

```yaml
type: custom:apexcharts-card
graph_span: 8h
header:
  show: true
  title: Smoke Temperature Last 8 Hour
  show_states: true
  colorize_states: true
all_series_config:
  stroke_width: 4
apex_config:
  dataLabels:
    enabled: true
yaxis:
  - max: 300
    min: 30
    apex_config:
      tickAmount: 9
series:
  - entity: sensor.esp_smoke_smoke_probe_1_min_alarm
    stroke_width: 1.5
    color: '#FA6057'
    opacity: 0.5
    curve: stepline
  - entity: sensor.esp_smoke_smoke_probe_1_temperature
    color: '#AD1007'
  - entity: sensor.esp_smoke_smoke_probe_1_max_alarm
    stroke_width: 1.5
    color: '#FA6057'
    opacity: 0.5
    curve: stepline
  - entity: sensor.esp_smoke_smoke_probe_2_min_alarm
    stroke_width: 1.5
    color: '#75FFAA'
    opacity: 0.5
    curve: stepline
  - entity: sensor.esp_smoke_smoke_probe_2_temperature
    color: '#15AD4F'
  - entity: sensor.esp_smoke_smoke_probe_2_max_alarm
    stroke_width: 1.5
    color: '#75FFAA'
    opacity: 0.5
    curve: stepline
```

And the result looks something like this:

![HA](https://github.com/stefslon/esphome-thermoworks-smoke/blob/main/imgs/HA.png)



## Resources

I found all these resources helpful in putting this together:

- https://hackaday.io/project/160386-blanket-the-smoke-signal-gateway
- http://yveaux.blogspot.com/2014/07/nrf24l01-sniffer-part-1.html
- http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html

