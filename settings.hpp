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
  
  uint32_t getMinVoltage();
  void setMinVoltage(double minv);
  
  uint32_t getMaxVoltage();
  void setMaxVoltage(double maxv);
  
  uint32_t getMinValue();
  void setMinValue(double minv);
  
  uint32_t getMaxValue();
  void setMaxValue(double maxv);

  size_t getName(char *dest, size_t len);
  void setName(char *name, size_t len);

  size_t getUnits(char *dest, size_t len);
  void setUnits(char *units, size_t len);
  
  void debugPrint();
  
private:
  struct {
    uint32_t uuid;  //  Unique identifier for this node
    double min_vol; //  Minimum voltage to be read
    double max_vol; //  Maxiumum voltage to be read
    double min_val; //  Minimum value that can be read
    double max_val; //  Maxiumum value that can be read
    char name[20];  //  Name of this node
    char units[10]; //  Unit symbol for the values to be read
  } _settings;
};

extern Configuration Config;
#endif
