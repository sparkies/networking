#include "packet.hpp"
#include "settings.hpp"

uint32_t packet_id = 0;

Packet::Packet() {
  _good = read();
}

Packet::Packet(uint32_t dest, byte *payload, size_t len)
  : origin(Config.getUUID()), dest(dest), id(packet_id++), len(len), _good(true) {
  data = new byte[len];
  memcpy(data, payload, len);
}

Packet::~Packet() {
  //  Since we allocate using new, we need to free the data with delete
  delete[] data;
}

uint32_t Packet::readU32(size_t offset) {
  return *(uint32_t *)(data + offset);
}

bool Packet::read() {
  if (!XBee.available()) {
    Serial.println(F("XBee not available."));
    return false;
  }

  //  Read in the origin
  if (XBee.readBytes((byte *)&origin, 4) != 4) {
    Serial.println(F("Could not read origin."));
    return false;
  }
  
  //  Read in the destination
  if (XBee.readBytes((byte *)&dest, 4) != 4) {
    Serial.println(F("Could not read destination."));
    return false;
  }
  
  //  Read in the packet id
  if (XBee.readBytes((byte *)&id, 4) != 4) {
    Serial.println(F("Could not read id."));
    return false;
  }
  
  //  Read in the length of the payload
  if (XBee.readBytes((byte *)&len, 1) != 1) {
    Serial.println(F("Could not read id."));
    return false;
  }

  //  If payload size is 0, then it's invalid
  if (!len) {
    Serial.println(F("Invalid length."));
    return false;
  }

  //  Allocate enough space for the payload
  data = new byte[len];
  
  //  Read in the payload
  if (XBee.readBytes(data, len) != len) {
    Serial.println(F("Could not read data."));
    return false;
  }
  
  return true;
}

void Packet::send() {
  //  +13 is for 3 longs (4 bytes each) + length byte
  byte payload[len + 13];

  //  Write each part of the header to its place in the payload
  //  This is so we can send the entire thing in one write call.
  memcpy(payload + Offset::Origin, (byte *)&origin, sizeof(origin));
  memcpy(payload + Offset::Destination, (byte *)&dest, sizeof(dest));
  memcpy(payload + Offset::PacketId, (byte *)&id, sizeof(id));
  payload[Offset::Length] = len;
  
  //  Copy payload to packet
  memcpy(payload + Offset::Payload, data, len);
  
  //  Send packet
  XBee.write(payload, sizeof(payload));
}

bool Packet::is_good() {
  return _good;
}

bool Packet::is_ours() {
  return dest == Config.getUUID();
}

void Packet::debugPrint() {
    Serial.println(F("Packet recv:"));
    Serial.print(F("Origin: "));
    Serial.println(origin, HEX);
    Serial.print(F("Destination: "));
    Serial.println(dest, HEX);
    Serial.print(F("Packet #: "));
    Serial.println(id, HEX);
    Serial.print(F("Data Length: "));
    Serial.println(len, DEC);
    Serial.print(F("Data: "));

    for (size_t i = 0; i < len; ++i) {
      if (i % 8 == 0) {
        Serial.println(F(""));
      }
      
      Serial.print(data[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println(F(""));
}

