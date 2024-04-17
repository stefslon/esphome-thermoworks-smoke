/*
 Copyright (C) 2024 S. Slonevskiy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.

 Uses RF24 Arduino library:
 https://www.arduino.cc/reference/en/libraries/rf24/
*/

#include <SPI.h>
#include <RF24.h>
#include <RF24_config.h>

#define RF_CE_PIN       (D4)
#define RF_CS_PIN       (D1)

// Might want to try 10, 40, 70 if nothing is captured on 10
#define RF_CHANNEL            (10)

#define RF_MAX_ADDR_WIDTH     (2)
#define MAX_RF_PAYLOAD_SIZE   (28) /* 5 address + 21 payload + 2 CRC */
#define SER_BAUDRATE          (115200)
#define PIPE                  (0)
#define RADIO_ID              ((uint64_t)0x55LL)

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
  radio.setDataRate((rf24_datarate_e)RF24_250KBPS);
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

    // Read the given CRC
    crc_given = ((uint16_t)(packet[payload_length + addr_len]) << 8) | (uint16_t)(packet[1 + payload_length + addr_len]);

    // Calculate the CRC
    crc = crc16(packet,payload_length + addr_len);

    if (crc == crc_given) {
      Serial.print("Address: ");
      dumpData(packet, addr_len);
      Serial.print(" Data: ");
      dumpData(packet+addr_len, payload_length);
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