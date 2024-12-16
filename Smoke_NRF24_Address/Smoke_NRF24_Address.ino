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
Uses RF24 Arduino library:
https://www.arduino.cc/reference/en/libraries/rf24/
*/

#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>

//------------------------------------------------------------------------------
// REFERENCE DATA: Examples of valid vs invalid readings
//------------------------------------------------------------------------------
/// @note Valid data example:
///   Address: 8F 43 3B 73 6F
///   Data: 8F 43 3B 73 6F BD 01 2C 06 40 01 BF 01 54 0B 40 01 01 00 00 00 01 00 00 96 00
///   Probe 1: 44.50°F (alarm min: 32.00, max: 158.00)
///   Probe 2: 44.70°F (alarm min: 32.00, max: 290.00)
/// @note False positive example:
///   Address: 5A 98 95 55 59 (non consistent address)
///   Data: 5A 98 95 55 59 49 65 F2 D4 DA AC B6 EA (short packet)
///   Probe 1: 2592.90 (alarm min: -2128.60, max: -1102.20)
///   Probe 2: -545.00 (alarm min: -2116.30, max: -1075.50)

//------------------------------------------------------------------------------
// SETUP INSTRUCTIONS
//------------------------------------------------------------------------------
/// When properly configured, this script should provide valid temperature
/// readings every 15 seconds. The readings should be within normal temperature
/// ranges as shown in the example above.
///
/// If you experience any of these issues:
/// - No readings appearing every 15 seconds
/// - Sporadic or missing data
/// - Temperature readings outside normal ranges
///
/// Try adjusting the configuration settings below. The most common adjustments
/// needed are:
/// 1. Pin assignments (RF_CE_PIN and RF_CS_PIN)
/// 2. Radio preamble (RADIO_ID)
///
/// @note Adjust all values in the CONFIGURATION section according to your setup


//------------------------------------------------------------------------------
// CONFIGURATION: These values need to be adjusted for your specific setup
//------------------------------------------------------------------------------

/// Pin number for the RF module's CE (Chip Enable) connection
/// @note Adjust this to match your wiring
#define RF_CE_PIN 4

/// Pin number for the RF module's CS (Chip Select/VSPI SS) connection
/// @note Adjust this to match your wiring
#define RF_CS_PIN 5

/// Radio preamble identifier
/// Must be 0xAA or 0x55 depending on the first bit of the address:
/// - Use 0xAALL for addresses starting with 0
/// - Use 0x55LL for addresses starting with 1
/// @note Configure based on your address configuration
/// @see https://travisgoodspeed.blogspot.com/2011/02/promiscuity-is-nrf24l01s-duty.html
#define RADIO_ID ((uint64_t)0xAALL)

/// RF channel number for communication
/// The device transmits on channels 10, 40, and 70 every 15 seconds.
/// @note Try different channels (10, 40, or 70) if no data is received
#define RF_CHANNEL 10

//------------------------------------------------------------------------------
// CONSTANTS: Do not modify these values
//------------------------------------------------------------------------------

#define RF_MAX_ADDR_WIDTH     (2)
#define MAX_RF_PAYLOAD_SIZE   (28) /* 5 address + 21 payload + 2 CRC */
#define SER_BAUDRATE          (115200)
#define PIPE                  (0)

// Set up nRF24L01 radio on SPI bus plus CE/CS pins
static RF24 radio(RF_CE_PIN, RF_CS_PIN);
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

// CRC-16/CCITT-FALSE
// https://gist.github.com/tijnkooijmans/10981093
uint16_t crc16(uint8_t* pData, int length)
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

void printc(uint8_t X) {
  if (X < 16) { Serial.print("0"); }
  Serial.print(X, HEX);
  Serial.print(" ");
}

inline static void dumpData(uint8_t* p, int len) {
  for (uint8_t i = 0; i < len; i++)
    printc(p[i]);
}

void setup() {
  Serial.begin(SER_BAUDRATE);

  radio.begin();

  // Disable shockburst
  radio.setAutoAck(false);
  radio.setRetries(0,0);

  // Match Smoke channel & datarate
  radio.setChannel(RF_CHANNEL);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(3, true);

  // Disable CRC & set fixed payload size to allow all packets captured to be returned by Nrf24.
  radio.disableCRC();
  radio.setPayloadSize(MAX_RF_PAYLOAD_SIZE);

  // Configure listening pipe with the 'promiscuous' address and start listening
  radio.setAddressWidth(RF_MAX_ADDR_WIDTH);
  radio.openReadingPipe(PIPE, RADIO_ID);
  radio.startListening();

  radio.printDetails();

}

void loop() {
  uint16_t crc, crc_given;
  uint8_t packet[MAX_RF_PAYLOAD_SIZE];
  uint8_t payload_length = 21;
  uint8_t addr_len = 5;
  data_t* tempData;

  if (radio.available()) {
    uint8_t packetLen = radio.getPayloadSize();

    if (packetLen > MAX_RF_PAYLOAD_SIZE)
      packetLen = MAX_RF_PAYLOAD_SIZE;

    radio.read(&packet, packetLen);

    for (uint8_t plen=3; plen <= 26; plen++) {

      // Read the given CRC
      crc_given = ((uint16_t)(packet[plen]) << 8) | (uint16_t)(packet[1 + plen]);

      // Calculate the CRC
      crc = crc16(packet,plen);

      if (crc == crc_given) {
        Serial.print("Address: ");
        dumpData(packet, addr_len);
        Serial.print("<- Note this address!>");
        Serial.println("");
        Serial.print(" Data: ");
        dumpData(packet, plen);
        Serial.println("");

        tempData = (data_t*)(packet+addr_len);
        Serial.print("     Probe 1: ");
        Serial.print((double)tempData->probe1_temp/10.0);
        Serial.print(" (alarm min: ");
        Serial.print((double)tempData->probe1_min/10.0);
        Serial.print(", max: ");
        Serial.print((double)tempData->probe1_max/10.0);
        Serial.println(")");

        Serial.print("     Probe 2: ");
        Serial.print((double)tempData->probe2_temp/10.0);
        Serial.print(" (alarm min: ");
        Serial.print((double)tempData->probe2_min/10.0);
        Serial.print(", max: ");
        Serial.print((double)tempData->probe2_max/10.0);
        Serial.println(")");

      }
    }
  }
}
