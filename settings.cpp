#include "settings.hpp"

Configuration Config;

Configuration::Configuration() {
  Serial.println("Loading configuration");
  EEPROM.get(0, _settings);
  debugPrint();
}

void Configuration::init() {
  Serial.println("Loading configuration");
  EEPROM.get(0, _settings);
  debugPrint();
}

void Configuration::flush() {
  EEPROM.put(0, _settings);
}

uint32_t Configuration::getUUID() {
  return _settings.uuid;
}

void Configuration::setUUID(uint32_t uuid) {
  _settings.uuid = uuid;
}

uint32_t Configuration::getMinV() {
  return _settings.min_v;
}

void Configuration::setMinV(double minv) {
  _settings.min_v = minv;
}

uint32_t Configuration::getMaxV() {
  return _settings.max_v;
}

void Configuration::setMaxV(double maxv) {
  _settings.max_v = maxv;
}

size_t Configuration::getName(char *dest, size_t dest_len) {
  size_t len = strlen(_settings.name);

  //  If destination length is shorter than strlen + null terminator
  if (dest_len < len + 1) {
    //  Then only copy up to dest_len bytes
    memcpy(dest, _settings.name, dest_len);
  } else {
    //  Otherwise, copy full name + trailing null
    memcpy(dest, _settings.name, len + 1);
  }
}

void Configuration::setName(char *name, size_t len) {
  //  Clear the name out before we set it.
  memset(_settings.name, 0, 100);
  
  //  Copy new name, but only up to 100 characters.
  memcpy(_settings.name, name, min(100, len));
}

void Configuration::debugPrint() {
  Serial.print("UUID: ");
  Serial.println(_settings.uuid);
  Serial.print("Name: ");
  Serial.println(_settings.name);
  Serial.print("Min Voltage: ");
  Serial.println(_settings.min_v);
  Serial.print("Max Voltage: ");
  Serial.println(_settings.max_v);
}
