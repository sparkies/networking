#ifndef CACHE_HPP
#define CACHE_HPP

#include <Arduino.h>

struct CacheEntry {
  uint32_t origin;
  uint32_t id;

  CacheEntry() : origin(0), id(0) {}
  CacheEntry(uint32_t orig, uint32_t id) : origin(orig), id(id) {}
};

class PacketCache {
public:
  PacketCache();
  PacketCache(size_t capacity);
  ~PacketCache();

  void insert(uint32_t origin, uint32_t id);
  bool has(uint32_t origin, CacheEntry *entry);
  void remove(uint32_t origin);
private:
  bool initialize(size_t capacity);
  bool resize(size_t capacity);

  size_t capacity;
  size_t size;
  CacheEntry *cache;
};

extern PacketCache Cache;

#endif

