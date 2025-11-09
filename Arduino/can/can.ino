// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <CAN.h>

#define CAN_RX_PIN 4
#define CAN_TX_PIN 5
#define CAN_BAUDRATE 500E3
#define EMUS_BASE 0x19B5

const uint32_t EMUS_VOLTAGE_ID = ((uint32_t) EMUS_BASE << 16) | 0x0001;


void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Receiver");
  CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);

  // start the CAN bus at 500 kbps
  if (!CAN.begin(CAN_BAUDRATE)) {
    Serial.println("Starting CAN failed!");
    while (true);
  }

  Serial.println("CAN started!");
}

void loop() {
  // sends voltage request then waits for 1 sec
  sendVoltageRequest();
  delay(1000);
  
  // parsing packet information
  int packetSize = CAN.parsePacket();
  if (!packetSize) return;

  long id = CAN.packetId();
  bool extended = CAN.packetExtended();
  bool rtr = CAN.packetRtr();
  int DLC = CAN.packetDlc();
  uint8_t data[8] = {0};



  // ignore rtr packets
  if (rtr) {
    while (CAN.available()) CAN.read();
    return;
  }

  int n = 0;
  while (CAN.available() && n < packetSize && n < 8) {
    data[n++] = CAN.read();
  }

  // received a packet
  Serial.print("Received ");
  Serial.print(extended ? "extended " : "standard ");
  Serial.print("packet with id 0x");
  Serial.print(id, HEX);

  // Interprets battery voltage information
  if (extended && id == EMUS_VOLTAGE_ID && DLC == 8) {
    const char *labels[] = {
      "MIN CELL VOLTAGE",
      "MAX CELL VOLTAGE",
      "AVERAGE CELL VOLTAGE",
      "TOTAL VOLTAGE (3rd byte)",
      "TOTAL VOLTAGE (LSB)",
      "TOTAL VOLTAGE (MSB)",
      "TOTAL VOLTAGE (2nd byte)",
      "RESERVED"
    };

    for (int i = 0; i < n; i++) {
      Serial.print(labels[i]);
      Serial.print(": ");
      Serial.println(data[i]);  // raw for now
    }
  } else {
    // Dump any other packet as raw information in hex
    for (int i = 0; i < n; i++) {
      if (data[i] < 0x10) Serial.print("0");
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

// Sends EMUS voltage request through CAN
void sendVoltageRequest() {
  if (CAN.beginExtendedPacket(EMUS_VOLTAGE_ID, 0, true)) {
    CAN.endPacket();
    Serial.print("TX ID 0x");
    Serial.print(EMUS_VOLTAGE_ID, HEX);
    Serial.println(" Voltage Request");
  }
}
