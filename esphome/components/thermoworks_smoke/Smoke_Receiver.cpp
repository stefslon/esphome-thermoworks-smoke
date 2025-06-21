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

/*
Uses RF24 Arduino library:
https://www.arduino.cc/reference/en/libraries/rf24/
*/


#include "Smoke_Receiver.h"
#include "esphome/core/log.h"

#define SMOKE_PAYLOAD_SIZE  21

#define RF_MAX_ADDR_WIDTH   2
#define MAX_RF_PAYLOAD_SIZE 28 /* 5 address + 21 payload + 2 CRC */
#define PIPE                0

#define RF_DATARATE         RF24_250KBPS
#define RF_ADDR_WIDTH       5
#define RF_CRC_LENGTH       2

/// Radio preamble identifier
/// Must be 0xAA or 0x55 depending on the first bit of the address:
/// - Use 0xAALL for addresses starting with 0
/// - Use 0x55LL for addresses starting with 1
/// @note Try both of these. Only one will find valid packets. This depends on the first bit of the address.
/// @see https://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html
#define RADIO_ID1 ((uint64_t)0xAALL)
#define RADIO_ID2 ((uint64_t)0x55LL)

namespace esphome {
namespace thermoworks_smoke {

static const char* const TAG = "thermoworks_smoke";

void SmokeReceiverComponent::setup() {
    ESP_LOGCONFIG(TAG, "Setting up RF24...");

    // Start nRF24 interface
    radio = RF24(get_pin_number(ce_pin), get_pin_number(cs_pin));

    if (!radio.begin()) {
        ESP_LOGE(TAG, "Failed to initialize RF24");
        this->mark_failed();
        return;
    }

    // Disable shockburst
    radio.setAutoAck(false);
    radio.setRetries(0,0);

    // Match Smoke channel & datarate
    radio.setChannel(rf_chan);
    radio.setDataRate((rf24_datarate_e)RF_DATARATE);
    
    // If ID is not defined start search mode
    if (this->radio_id==0) {
        // Change polling interval to 2 seconds
        this->set_update_interval(2000); 
        
        radio.setPALevel(3, true);
        
        // Disable CRC & set fixed payload size to allow all packets captured to be returned by Nrf24.
        radio.disableCRC();
        radio.setPayloadSize(MAX_RF_PAYLOAD_SIZE);
        
        // Configure listening pipe with the 'promiscuous' address and start listening
        radio.setAddressWidth(RF_MAX_ADDR_WIDTH);
        if (use_alt_id==0)
            radio.openReadingPipe(PIPE, RADIO_ID1);
        else
            radio.openReadingPipe(PIPE, RADIO_ID2);
    }
    else {
        // Set fixed payload size to allow all packets captured to be returned by Nrf24.
        radio.setPayloadSize(SMOKE_PAYLOAD_SIZE);
        
        // Configure listening pipe with the 'promiscuous' address and start listening
        radio.setAddressWidth(RF_ADDR_WIDTH);
        radio.openReadingPipe(PIPE, radio_id);
    }

    radio.startListening();

    ESP_LOGCONFIG(TAG,"Setup executed, started listening...");
}

void SmokeReceiverComponent::update() {
    data_t packet;
    if (radio.available()) {
        uint8_t packetLen = radio.getPayloadSize();
        // If ID is not defined start search mode
        if (this->radio_id==0) {
            ESP_LOGI(TAG, "Searching for Smoke unit...");
            uint16_t crc, crc_given;
            uint8_t packet[MAX_RF_PAYLOAD_SIZE];
            uint8_t addr_len = 5;
            data_t* tempData;
            
            if (packetLen > MAX_RF_PAYLOAD_SIZE)
              packetLen = MAX_RF_PAYLOAD_SIZE;

            radio.read(&packet, packetLen);

            for (uint8_t plen=3; plen <= 26; plen++) {

              // Read the given CRC
              crc_given = ((uint16_t)(packet[plen]) << 8) | (uint16_t)(packet[1 + plen]);

              // Calculate the CRC
              crc = crc16(packet,plen);

              if (crc == crc_given) {
                char ADDR[16];
                snprintf(ADDR,16,"0x%02X%02X%02X%02X%02X",packet[0],packet[1],packet[2],packet[3],packet[4]);
                
                ESP_LOGI(TAG, "   Address: %s <-- use this address for radio_id field", ADDR);
                ESP_LOGI(TAG, "   Received data: ");
                
                tempData = (data_t*)(packet+addr_len);
                                
                char tempStr[16];
                char T = 'F';

                if (tempData->units==0) T = 'C'; /* temperature is in C */
                
                snprintf(tempStr,16,"%.1f°%c",(float)tempData->probe1_temp/10.0,T);
                if (tempData->probe1) snprintf(tempStr,16,"---");

                ESP_LOGI(TAG, "       Probe 1: %s (alarm min: %.1f°%c, max %.1f°%c) ",tempStr,
                    (float)tempData->probe1_min/10.0,T,
                    (float)tempData->probe1_max/10.0,T);
                    
                snprintf(tempStr,16,"%.1f°%c",(float)tempData->probe2_temp/10.0,T);
                if (tempData->probe2) snprintf(tempStr,16,"---");
                    
                ESP_LOGI(TAG, "       Probe 2: %s (alarm min: %.1f°%c, max %.1f°%c) ",tempStr,
                    (float)tempData->probe2_min/10.0,T,
                    (float)tempData->probe2_max/10.0,T);
              }
            }
                    
        }
        else {
            if (packetLen > SMOKE_PAYLOAD_SIZE)
                packetLen = SMOKE_PAYLOAD_SIZE;

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

            ESP_LOGD(TAG,"Raw data received: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", packet.probe1_temp, packet.probe1_max, packet.probe1_min, packet.probe2_temp, packet.probe2_max, packet.probe2_min, packet.alarm1, packet.probe1, packet.alarm2, packet.probe2, packet.units);
        }
    }
}

void SmokeReceiverComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "Smoke Receiver Component");
    LOG_PIN("  CE Pin: ", ce_pin);
    LOG_PIN("  CS Pin: ", cs_pin);
    if (radio_id==0) {
        ESP_LOGCONFIG(TAG, "Mode: SEARCH MODE");
    }
    else {
        ESP_LOGCONFIG(TAG, "Mode: RECEIVER MODE");
        ESP_LOGCONFIG(TAG, "Address: 0x%" PRIX64, radio_id);
    }
}

uint8_t SmokeReceiverComponent::get_pin_number(GPIOPin *pin) {
    // Cast to InternalGPIOPin to get the actual pin number
    auto *internal_pin = static_cast<esphome::InternalGPIOPin*>(pin);
    return internal_pin->get_pin();
}

// CRC-16/CCITT-FALSE
// https://gist.github.com/tijnkooijmans/10981093
uint16_t SmokeReceiverComponent::crc16(uint8_t* pData, int length)
{
    uint8_t i;
    uint16_t wCrc = 0xffff;
    while (length--) {
        wCrc ^= *(unsigned char *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}

}  // namespace thermoworks_smoke
}  // namespace esphome
