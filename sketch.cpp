#include <Arduino.h>
#include <SoftwareSerial.h>

#define DEBUG
#define LOG
#include "log.hpp"

#include "cache.hpp"
#include "packet.hpp"
#include "settings.hpp"

#define NODE_NAME       "Light Endpoint 1"
#define NODE_UNITS      ""
#define NODE_UUID       3
#define NODE_MINVOLTAGE 0
#define NODE_MAXVOLTAGE 5
#define NODE_MINVALUE   0
#define NODE_MAXVALUE   0

/// Broadcast data to each of the given nodes.
/// Only used when NodeType is Sender (i.e., send to server and endpoint node)
const uint32_t NODE_BROADCAST[] = {0, 3};
/// Broadcast data to a given node.
/// Only used for endpoint nodes to send data to the node they are linked to.
const uint32_t NODE_DIRECT[] = {2};

const uint8_t NODE_ANALOG_PIN = 0;
const uint32_t BROADCAST_TIMEOUT = 1000;

enum class NodeType {
  Relay,    //  Nodes that just relay packets
  Endpoint, //  Nodes that take in data and process it
  Sender    //  Nodes that read data and send it
};

#define NODE_TYPE       NodeType::Endpoint

/// Data for light configuration
const int LIGHT_PIN = 8;
const int LIGHT_TIMEOUT = 1000;

bool broadcasting = true;
bool light_on = false;
uint32_t last_millis = 0;

bool can_send(uint32_t last_check);
void init_config();
void handleXBee();
void handlePacket(Packet *packet);

static Packet inbound;

// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX

int main() {
  init();
  
  //  Start serial ports
  Serial.begin(9600);
  XBee.begin(9600);

  init_config();

  uint32_t timeout = 1 * 1000; // Wait 60 seconds per sample
  uint32_t last_check = millis();
  Packet *response = new Packet();
  
  for (;;) {
    //  Serial passthrough to XBee
    if (Serial.available()) {
      XBee.print(Serial.readString());
    }
    
    //  Handle updates through XBee
    handleXBee();

    if (NODE_TYPE == NodeType::Endpoint) {
      digitalWrite(LIGHT_PIN, light_on ? HIGH : LOW);
    }

    if (can_send(last_check)) {
      if (NODE_TYPE == NodeType::Endpoint) {
        logmsg(F("Sending light reading."));
        last_check = millis();

        for (auto&& node : NODE_DIRECT) {
          response->send_to(node, reinterpret_cast<uint8_t *>(&light_on), sizeof(uint8_t));
        }
        
        if (light_on && last_check - last_millis > LIGHT_TIMEOUT) {
          light_on = false;
        }
      } else if (NODE_TYPE == NodeType::Sender) {
        logmsg(F("Sending analog reading"));
        last_check = millis();
        int reading = analogRead(NODE_ANALOG_PIN);
        
        for (auto&& node : NODE_BROADCAST) {
          response->send_to(node, reinterpret_cast<uint8_t *>(&reading), sizeof(int));
        }
      }
    }
  }
}

bool can_send(uint32_t last_check) {
  return last_check + BROADCAST_TIMEOUT < millis();
}

void init_config() {
  Config.setName(NODE_NAME, strlen(NODE_NAME));
  Config.setUUID(NODE_UUID);
  Config.setMinVoltage(NODE_MINVOLTAGE);
  Config.setMaxVoltage(NODE_MAXVOLTAGE);
  Config.setMinValue(NODE_MINVALUE);
  Config.setMaxValue(NODE_MAXVALUE);
  Config.setUnits(NODE_UNITS, strlen(NODE_UNITS));
  Config.flush();

  #ifdef DEBUG
    Config.debugPrint();
  #endif
}

void handleXBee() {
  if (XBee.available()) {
    if (!inbound.read()) {
      logmsg(F("Packet wasn't good."));
      return;
    }

    #ifdef DEBUG
      inbound.debugPrint();
    #endif

    CacheEntry entry;
    
    if (Cache.has(inbound.origin, &entry)) {
      if (entry.id >= inbound.id) {
        logmsg(F("Ignoring old packet."));
        return;
      } else {
        entry.id = inbound.id;
      }
    } else {
      logmsg(F("New origin found."));
      Cache.insert(inbound.origin, inbound.id);
    }

    if (inbound.is_ours()) {
      logmsg(F("Received packet."));
      handlePacket(&inbound);
    } else if (inbound.is_global()) {
      logmsg(F("Received global packet."));
      handlePacket(&inbound);
      inbound.send();
    } else if (inbound.is_same_origin()) {
      logmsg(F("Same origin; dropping."));
    } else {
      logmsg(F("Not our problem."));
      inbound.send();
    }
  }
}

void handlePacket(Packet *packet) {
  static Packet *outbound = new Packet();
  static bool init_once = true;
  static uint8_t *configuration;
  static uint8_t config_size;

  if (NODE_TYPE == NodeType::Endpoint) {
    light_on = false;
    for (auto&& id : NODE_DIRECT) {
      if (id == packet->origin) {
        light_on = true;
      }
    }
    last_millis = millis();
  }

  //  Configuration response
  if (packet->len == 1 && packet->data[0] == 'C') {
    //  If never initialized, initialize it now
    if (init_once) {
      config_size = Config.serialize(configuration);
      init_once = false;
    }
    //  Send configuration to origin
    outbound->send_to(packet->origin, configuration, config_size);
  } else if (packet->len == 1 && packet->data[0] == 'B') {
    broadcasting = true;
  } else if (packet->len == 1 && packet->data[0] == 'S') {
    broadcasting = false;
  } else if (packet->len == 1 && packet->data[0] == 'P') {
    char *payload = "OK";
    outbound->send_to(packet->origin, payload, 2);
  }
}

