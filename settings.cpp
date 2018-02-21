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

uint32_t Configuration::getMinVoltage() {
  return _settings.min_vol;
}

void Configuration::setMinVoltage(double minv) {
  _settings.min_vol = minv;
}

uint32_t Configuration::getMaxVoltage() {
  return _settings.max_vol;
}

void Configuration::setMaxVoltage(double maxv) {
  _settings.max_vol = maxv;
}

uint32_t Configuration::getMinValue() {
  return _settings.min_val;
}

void Configuration::setMinValue(double minv) {
  _settings.min_val = minv;
}

uint32_t Configuration::getMaxValue() {
  return _settings.max_val;
}

void Configuration::setMaxValue(double maxv) {
  _settings.max_val = maxv;
}

size_t Configuration::getName(char *dest, size_t dest_len) {
  size_t len = strlen(_settings.name);

  //  If destination length is shorter than strlen + null terminator
  if (dest_len < len + 1) {
    //  Then only copy up to dest_len bytes
    memcpy(dest, _settings.name, dest_len);
    return dest_len;
  } else {
    //  Otherwise, copy full name + trailing null
    memcpy(dest, _settings.name, len + 1);
    return len + 1;
  }
}

void Configuration::setName(char *name, size_t len) {
  //  Clear the name out before we set it.
  memset(_settings.name, 0, sizeof(_settings.name));
  
  //  Copy new name, but only up to 100 characters.
  memcpy(_settings.name, name, min(100, len));
}

size_t Configuration::getUnits(char *dest, size_t dest_len) {
  size_t len = strlen(_settings.units);

  //  If destination length is shorter than strlen + null terminator
  if (dest_len < len + 1) {
    //  Then only copy up to dest_len bytes
    memcpy(dest, _settings.units, dest_len);
    return dest_len;
  } else {
    //  Otherwise, copy full name + trailing null
    memcpy(dest, _settings.units, len + 1);
    return len + 1;
  }
}

void Configuration::setUnits(char *units, size_t len) {
  //  Clear the units out before we set them.
  memset(_settings.units, 0, sizeof(_settings.units));
  
  //  Copy new name, but only up to 100 characters.
  memcpy(_settings.units, units, min(10, len));
}

void Configuration::debugPrint() {
  Serial.print("UUID: ");
  Serial.println(_settings.uuid);
  Serial.print("Name: ");
  Serial.println(_settings.name);
  Serial.print("Min Voltage: ");
  Serial.println(_settings.min_vol);
  Serial.print("Max Voltage: ");
  Serial.println(_settings.max_vol);
  Serial.print("Min Value: ");
  Serial.println(_settings.min_val);
  Serial.print("Max Value: ");
  Serial.println(_settings.max_val);
  Serial.print("Units: ");
  Serial.println(_settings.units);
}
