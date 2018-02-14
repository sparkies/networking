#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <Arduino.h>
#include <EEPROM.h>

class Configuration {
public:
  Configuration();
  void init();
  void flush();
  
  uint32_t getUUID();
  void setUUID(uint32_t uuid);
  
  uint32_t getMinV();
  void setMinV(double minv);
  
  uint32_t getMaxV();
  void setMaxV(double maxv);

  size_t getName(char *dest, size_t len);
  void setName(char *name, size_t len);
  
  void debugPrint();
  
private:
  struct {
    uint32_t uuid;
    double min_v;
    double max_v;
    char name[100];
    char units[10];
  } _settings;
};

extern Configuration Config;
#endif
