#include <Arduino.h>
#include <SoftwareSerial.h>

#include "cache.hpp"
#include "packet.hpp"
#include "settings.hpp"

const int LIGHT_PIN = 8;

bool broadcasting = true;
bool light_on = false;
uint32_t last_millis = 0;

void handleXBee();
void handlePacket(Packet *packet);

// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX

int main() {
  init();
  
  //  Start serial ports
  Serial.begin(9600);
  XBee.begin(9600);

//  uint32_t relay_ids[] = {0, 3};
//  Config.setRelay(relay_ids, 2);
//  Config.setName("Test 2", 5);
//  Config.setUUID(3);
//  Config.setMinVoltage(0);
//  Config.setMaxVoltage(5);
//  Config.setMinValue(0);
//  Config.setMaxValue(150);
//  Config.setUnits("C", 2);
//  Config.flush();

  Config.debugPrint();

  uint32_t timeout = 1 * 1000; // Wait 60 seconds per sample
  uint32_t last_check = millis();
  
  for (;;) {
    //  Serial passthrough to XBee
    if (Serial.available()) {
      XBee.print(Serial.readString());
    }
    
    //  Handle updates through XBee
    handleXBee();

    digitalWrite(LIGHT_PIN, light_on ? HIGH : LOW);

    if (broadcasting && last_check + timeout < millis()) {
      Serial.println("Sending reading.");
      last_check = millis();
      Packet response(2, reinterpret_cast<uint8_t *>(&light_on), sizeof(uint8_t));
      response.send();

      if (light_on && last_check - last_millis > 5000) {
        light_on = false;
      }
    }
  }
}

void handleXBee() {
  if (XBee.available()) {
    Packet packet;
    if (!packet.is_good()) {
      Serial.println(F("Packet wasn't good."));
      return;
    }

    packet.debugPrint();

    CacheEntry entry;
    
    if (Cache.has(packet.origin, &entry)) {
      if (entry.id >= packet.id) {
        Serial.println(F("Ignoring old packet."));
        return;
      } else {
        entry.id = packet.id;
      }
    } else {
      Cache.insert(packet.origin, packet.id);
    }

    if (packet.is_ours()) {
      Serial.println(F("Received packet."));
      handlePacket(&packet);
    } else if (packet.is_global()) {
      Serial.println(F("Received global packet."));
      handlePacket(&packet);
      packet.send();
    } else if (packet.is_same_origin()) {
      Serial.println(F("Same origin; dropping."));
    } else {
      Serial.println(F("Not our problem."));
      packet.send();
    }
  }
}

void handlePacket(Packet *packet) {
  if (packet->origin == 2 && packet->data[0] == '1') {
    last_millis = millis();
    if (!light_on) {
      light_on = true;
      digitalWrite(LIGHT_PIN, light_on ? HIGH : LOW);
    }
  }
  //  Configuration response
  else if (packet->len == 1 && packet->data[0] == 'C') {
    uint8_t *payload = nullptr;
    size_t size = Config.serialize(payload);
    Packet response(packet->origin, payload, size);
    response.send();
  } else if (packet->len == 1 && packet->data[0] == 'B') {
    broadcasting = true;
  } else if (packet->len == 1 && packet->data[0] == 'S') {
    broadcasting = false;
  } else if (packet->len == 1 && packet->data[0] == 'I') {
    byte *output;
    int length = Config.serialize(output);
    Packet response(packet->origin, output, length);
    response.send();
  } else if (packet->len == 1 && packet->data[0] == 'P') {
    byte payload[] = "OK";
    Packet response(packet->origin, payload, sizeof(payload));
    response.send();
  }
}

