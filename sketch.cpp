#include <Arduino.h>
#include <SoftwareSerial.h>

#include "cache.hpp"
#include "packet.hpp"
#include "settings.hpp"

void handleXBee();

// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX

int main() {
  init();
  
  //  Start serial ports
  Serial.begin(9600);
  XBee.begin(9600);

//  Config.setName("Test", 5);
//  Config.setUUID(2);
//  Config.setMinVoltage(0);
//  Config.setMaxVoltage(5);
//  Config.setMinValue(0);
//  Config.setMaxValue(150);
//  Config.setUnits("C", 2);
//  Config.flush();

  Config.debugPrint();

  for (;;) {
    //  Serial passthrough to XBee
    if (Serial.available()) {
      XBee.print(Serial.readString());
    }
    
    //  Handle updates through XBee
    handleXBee();
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
      byte payload[] = "Got it.";
      Packet response(packet.origin, payload, sizeof(payload));
      response.send();
    } else {
      Serial.println(F("Not our problem."));
      packet.send();
    }
  }
}


