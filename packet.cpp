#include "log.hpp"
#include "packet.hpp"
#include "settings.hpp"

uint32_t packet_id = 0;

Packet::Packet() : origin(Config.getUUID()), dest(0), id(packet_id++), len(0), _good(false) {
}

Packet::Packet(uint32_t dest)
  : origin(Config.getUUID()),
   dest(dest), id(packet_id++), 
   len(0), _good(true) 
{
}

Packet::Packet(uint32_t dest, byte *payload, size_t len)
  : origin(Config.getUUID()), dest(dest), id(packet_id++), len(len), _good(true) {
  memcpy(data, payload, len);
}

Packet::~Packet() {
}

uint32_t Packet::readU32(size_t offset) {
  return *(uint32_t *)(data + offset);
}

bool Packet::read() {
  if (!XBee.available()) {
    //Serial.println(F("XBee not available."));
    return false;
  }

  int tries = 0;
  bool searching = true;
  int last = 0;
  while (searching && tries < 100) {
    int val = 0;
    
    if (last == START_BYTES[0]) {
      val = last;
    } else {
      val = XBee.read();
    }

    if (val == START_BYTES[0]) {
      int second = XBee.read();

      if (second == START_BYTES[1]) {
        searching = false;
      } else {
        last = second;
      }
    } else {
      last = 0;
    }
    
    tries++;
  }

  if (searching) {
    Serial.println(F("Could not read after 100 tries."));
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
  
  //  Read in the packet checksum
  if (XBee.readBytes((byte *)&checksum, 2) != 2) {
    Serial.println(F("Could not read checksum."));
    return false;
  }
  
  //  Read in the length of the payload
  if (XBee.readBytes((byte *)&len, 1) != 1) {
    Serial.println(F("Could not read id."));
    return false;
  }

  //  If payload size is 0, then it's invalid
  if (!len) {
    //Serial.println(F("Invalid length."));
    return false;
  }
  
  //  Read in the payload
  if (XBee.readBytes(data, len) != len) {
    Serial.println(F("Could not read data."));
    return false;
  }
  
  int sum = calculateChecksum();
  
  if (sum != checksum) {
    return false;
  }

  return true;
}

void Packet::send(bool calculate_checksum) {
  if (calculate_checksum) {
    checksum = calculateChecksum();
  }

  if (checksum != calculateChecksum()) {
    Serial.println(F("Not sending because it would be garbage."));
    return;
  }
  
  //  +16 is for start bytes, 3 longs (4 bytes each), checksum, and length byte
  byte payload[len + 17];
  payload[0] = START_BYTES[0];
  payload[1] = START_BYTES[1];

  //  Write each part of the header to its place in the payload
  //  This is so we can send the entire thing in one write call.
  memcpy(payload + Offset::Origin, (byte *)&origin, sizeof(origin));
  memcpy(payload + Offset::Destination, (byte *)&dest, sizeof(dest));
  memcpy(payload + Offset::PacketId, (byte *)&id, sizeof(id));
  memcpy(payload + Offset::Checksum, (byte *)&checksum, sizeof(checksum));
  payload[Offset::Length] = len;
  
  //  Copy payload to packet
  memcpy(payload + Offset::Payload, data, len);
  
  //  Send packet
  XBee.write(payload, len + 17);
}

void Packet::send_to(uint32_t dest, byte *payload, size_t length) {
  if (length > 255) {
    logmsg(F("Invalid packet length."));
  }
  this->dest = dest;
  this->len = length;
  this->id = packet_id++;

  memcpy(data, payload, len);
  this->send();
}

bool Packet::is_good() {
  return _good;
}

bool Packet::is_global() {
  return dest == Packet::Any;
}

bool Packet::is_ours() {
  return dest == Config.getUUID();
}

bool Packet::is_same_origin() {
  return origin == Config.getUUID();
}

void Packet::debugPrint() {
    Serial.println(F("Packet recv:"));
    Serial.print(F("Origin: "));
    Serial.println(origin, HEX);
    Serial.print(F("Destination: "));
    Serial.println(dest, HEX);
    Serial.print(F("Packet #: "));
    Serial.println(id, HEX);
    Serial.print(F("Checksum: "));
    Serial.println(calculateChecksum(), HEX);
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

uint16_t Packet::calculateChecksum() {
  
  uint16_t sum = origin + dest + id + len;

  for (int i = 0; i < len; ++i) {
    sum += data[i];
  }

  return sum;
}

