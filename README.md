## Introduction

These files provide ESPHome to Home Assitant integration of Thermoworks' [Smoke® Remote BBQ Alarm Thermometer](https://www.thermoworks.com/smoke/). I use these to get Smoke temperature data into Home Assistant dashboard.

### You will need:
- nRF24L01 Transceiver Module
- ESPHome capable microcontroller (e.g. ESP-12F D1 or ESP8266)

### Steps:
- Wire up nRF24L01 to microcontroller (there are plenty of tutorials out there)
- Find RADIO_ID - this is the tricky part and will require some trial and error, use [NRF24_Sniffer](https://github.com/Yveaux/NRF24_Sniffer) what ID your Thermoworks unit is using 
- Update Smoke_Receiver.h with the appropriate value
- Copy Smoke_Receiver.h to /homeassistant/esphome
- Create ESPHome integration (use esp-smoke.yaml for guidance)
- Setup dashboard in Home Assitant (I used [ApexCharts Card](https://github.com/RomRider/apexcharts-card) for graphs)

## Resources

I found all these resources helpful in putting this together:

- https://hackaday.io/project/160386-blanket-the-smoke-signal-gateway
- http://yveaux.blogspot.com/2014/07/nrf24l01-sniffer-part-1.html
- http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html

