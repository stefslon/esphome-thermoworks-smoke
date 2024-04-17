/*
MIT License

Copyright (c) 2024 Stefan Slonevskiy

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

/*
    Copy this file to /homeassistant/esphome
*/

#include "esphome.h"

#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>

#define MAX_RF_PAYLOAD_SIZE    (21)
#define PIPE                   (0)

#define RF_CHANNEL             (10)
#define RF_DATARATE            (RF24_250KBPS)
#define RF_ADDR_WIDTH          (5)
#define RADIO_ID               ((uint64_t)0x123456789ALL) /* Need to be set to whatever your unit's ID is */
#define RF_CRC_LENGTH          (2)

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

class Smoke_Receiver : public PollingComponent, public Sensor {
    public:

    // Set up nRF24L01 radio on SPI bus plus CE/CS pins
    RF24 radio;

    Sensor *probe1_temp = new Sensor();
    Sensor *probe1_min = new Sensor();
    Sensor *probe1_max = new Sensor();
    Sensor *probe2_temp = new Sensor();
    Sensor *probe2_min = new Sensor();
    Sensor *probe2_max = new Sensor(); 

    // constructor
    Smoke_Receiver() : PollingComponent(15000) {}

    float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

    void setup() override {
        radio = RF24(D4, D1);

        radio.begin();

        // Disable shockburst
        radio.setAutoAck(false);
        radio.setRetries(0,0);

        // Match Smoke channel & datarate
        radio.setChannel(RF_CHANNEL);
        radio.setDataRate((rf24_datarate_e)RF_DATARATE);

        // Disable CRC & set fixed payload size to allow all packets captured to be returned by Nrf24.
        radio.setPayloadSize(MAX_RF_PAYLOAD_SIZE);

        // Configure listening pipe with the 'promiscuous' address and start listening
        radio.setAddressWidth(RF_ADDR_WIDTH);
        radio.openReadingPipe(PIPE, RADIO_ID);
        radio.startListening();

        ESP_LOGI("esp-smoke","Setup executed, started listening...");
    }
    
    void update() override {
        data_t packet;
        if (radio.available()) {
            uint8_t packetLen = radio.getPayloadSize();

            if (packetLen > MAX_RF_PAYLOAD_SIZE)
                packetLen = MAX_RF_PAYLOAD_SIZE;

            radio.read(&packet, packetLen);

            float temp_scale = 0.1;
            float temp_offset = 0.0;
            if (packet.units==0) { /* temperature is in C */
                temp_scale = 0.1*9.0/5.0;
                temp_offset = 32.0;
            }

            if (packet.probe1==0)
                probe1_temp->publish_state((float)packet.probe1_temp*temp_scale+temp_offset);
                
            probe1_min->publish_state((float)packet.probe1_min*temp_scale+temp_offset);
            probe1_max->publish_state((float)packet.probe1_max*temp_scale+temp_offset);

            if (packet.probe2==0)
                probe2_temp->publish_state((float)packet.probe2_temp*temp_scale+temp_offset);
                
            probe2_min->publish_state((float)packet.probe2_min*temp_scale+temp_offset);
            probe2_max->publish_state((float)packet.probe2_max*temp_scale+temp_offset);

            ESP_LOGI("esp-smoke","Data received: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", packet.probe1_temp, packet.probe1_max, packet.probe1_min, packet.probe2_temp, packet.probe2_max, packet.probe2_min, packet.alarm1, packet.probe1, packet.alarm2, packet.probe2, packet.units);
        }
    }

};

