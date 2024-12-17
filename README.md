## Introduction

These files provide ESPHome to Home Assitant integration of Thermoworks' [Smoke® Remote BBQ Alarm Thermometer](https://www.thermoworks.com/smoke/). I use this integration to get Smoke temperature data into Home Assistant dashboard without Smoke® Gateway (Wi-Fi bridge).

### You will need:
- nRF24L01 Transceiver Module
- ESPHome capable microcontroller (e.g. ESP-12F D1 or ESP8266)

### Steps:
1. Wire up nRF24L01 to microcontroller (there are plenty of tutorials out there, [here](https://projecthub.arduino.cc/tmekinyan/how-to-use-the-nrf24l01-module-with-arduino-813957) is one example)
2. Find RADIO_ID - this is the tricky part and will require some trial and error, use provided [Smoke_NRF24_Address.ino](https://github.com/stefslon/esphome-thermoworks-smoke/blob/main/Smoke_NRF24_Address/Smoke_NRF24_Address.ino) Arduino sketch to scan for Smoke transmissions. You might want to try changing channels to 40 and 70. If valid transmission is found then address of that transmission along with decoded temperature data is printed out.
3. Update Smoke_Receiver.h with the appropriate values for your address as well as your pinouts.
4. Copy Smoke_Receiver.h to /homeassistant/esphome
5. Create ESPHome integration (use provided esp-smoke.yaml for guidance)
6. Setup dashboard in Home Assitant (I used [ApexCharts Card](https://github.com/RomRider/apexcharts-card) for graphs)

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
![HA](https://github.com/stefslon/esphome-thermoworks-smoke/assets/2256156/e78d3913-6996-467f-bd00-7663fb4b56d3)



## Resources

I found all these resources helpful in putting this together:

- https://hackaday.io/project/160386-blanket-the-smoke-signal-gateway
- http://yveaux.blogspot.com/2014/07/nrf24l01-sniffer-part-1.html
- http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html

