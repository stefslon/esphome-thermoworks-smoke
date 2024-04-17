## Introduction

These files provide ESPHome to Home Assitant integration of Thermoworks' [Smoke® Remote BBQ Alarm Thermometer](https://www.thermoworks.com/smoke/). I use this integration to get Smoke temperature data into Home Assistant dashboard without Smoke® Gateway (Wi-Fi bridge).

### You will need:
- nRF24L01 Transceiver Module
- ESPHome capable microcontroller (e.g. ESP-12F D1 or ESP8266)

### Steps:
1. Wire up nRF24L01 to microcontroller (there are plenty of tutorials out there, [here](https://projecthub.arduino.cc/tmekinyan/how-to-use-the-nrf24l01-module-with-arduino-813957) is one example)
2. Find RADIO_ID - this is the tricky part and will require some trial and error, use provided [Smoke_NRF24_Address.ino](https://github.com/stefslon/esphome-thermoworks-smoke/blob/main/Smoke_NRF24_Address.ino) Arduino sketch to scan for Smoke transmissions. You might want to try changing channels to 40 and 70. If valid transmission is found then address of that transmission along with decoded temperature data is printed out.
3. Update Smoke_Receiver.h with the appropriate value
4. Copy Smoke_Receiver.h to /homeassistant/esphome
5. Create ESPHome integration (use provided esp-smoke.yaml for guidance)
6. Setup dashboard in Home Assitant (I used [ApexCharts Card](https://github.com/RomRider/apexcharts-card) for graphs)

## Resources

I found all these resources helpful in putting this together:

- https://hackaday.io/project/160386-blanket-the-smoke-signal-gateway
- http://yveaux.blogspot.com/2014/07/nrf24l01-sniffer-part-1.html
- http://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html

