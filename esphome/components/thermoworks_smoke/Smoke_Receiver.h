/*
MIT License

Copyright (c) 2024-2025 Stefan Slonevskiy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/sensor/sensor.h"

#include <RF24.h>
#include <RF24_config.h>

namespace esphome {
namespace thermoworks_smoke {

typedef struct _data_t
{
    int16_t probe1_temp;
    int16_t probe1_max;
    int16_t probe1_min;
    int16_t probe2_temp;
    int16_t probe2_max;
    int16_t probe2_min;
    int8_t alarm1;
    int8_t probe1;
    int8_t alarm2;
    int8_t probe2;
    int8_t units;
    int8_t sync;
    int8_t dummy[3];
} data_t;

class SmokeReceiverComponent : public PollingComponent {

    protected:
    
        sensor::Sensor *probe1_temp;
        sensor::Sensor *probe1_min;
        sensor::Sensor *probe1_max;
        sensor::Sensor *probe2_temp;
        sensor::Sensor *probe2_min;
        sensor::Sensor *probe2_max; 
        
        // Set up nRF24L01 radio on SPI bus plus CE/CS pins
        RF24 radio;
        
        GPIOPin *cs_pin;
        GPIOPin *ce_pin;
        uint64_t radio_id = 0;
        uint8_t rf_chan;
        uint8_t use_alt_id = 0;
      
    public:
    
        void set_probe1_temp_sensor(sensor::Sensor *s) { probe1_temp = s; }
        void set_probe1_min_sensor(sensor::Sensor *s) { probe1_min = s; }
        void set_probe1_max_sensor(sensor::Sensor *s) { probe1_max = s; }
        void set_probe2_temp_sensor(sensor::Sensor *s) { probe2_temp = s; }
        void set_probe2_min_sensor(sensor::Sensor *s) { probe2_min = s; }
        void set_probe2_max_sensor(sensor::Sensor *s) { probe2_max = s; }
    
        void set_cs_pin(GPIOPin *p) { cs_pin = p; }
        void set_ce_pin(GPIOPin *p) { ce_pin = p; }
        void set_rf_chan(uint8_t p) { rf_chan = p; }
        void set_radio_id(uint64_t p) { radio_id = p; }
        void set_use_alt_id(uint8_t p) { use_alt_id = p; }

        // constructor
        //Smoke_Receiver() : PollingComponent(15000) {}

        void setup() override;
        void update() override;
        void dump_config() override;
        
    private:
    
        uint8_t get_pin_number(GPIOPin *pin);
        uint16_t crc16(uint8_t* pData, int length);

};

}  // namespace thermoworks_smoke
}  // namespace esphome
