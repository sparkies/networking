#include "cache.hpp"

PacketCache Cache;

PacketCache::PacketCache() : capacity(0), size(0), cache(nullptr) {}

PacketCache::PacketCache(size_t capacity) : size(0) {
  initialize(capacity);
}

PacketCache::~PacketCache() {
  if (cache != nullptr) {
    free(cache);
  }
}

void PacketCache::insert(uint32_t origin, uint32_t id) {
  if (cache == nullptr) {
    initialize(10);
  }

  //  If we hit the capacity, then increase total size by 5
  if (size >= capacity) {
    resize(capacity + 5);
  }

  CacheEntry *entry;

  cache[size] = CacheEntry(origin, id);
  size++;
}

bool PacketCache::has(uint32_t origin, CacheEntry *entry) {
  for (size_t i = 0; i < size; ++i) {
    if (cache[i].origin == origin) {
      entry = &cache[i];
      return true;
    }
  }

  entry = nullptr;
  return false;
}

void PacketCache::remove(uint32_t origin) {
  
}

bool PacketCache::initialize(size_t cap) {
  //  Create the PacketCache with the given capacity
  cache = static_cast<CacheEntry *>(calloc(cap, sizeof(CacheEntry)));

  //  If allocation failed, return false
  if (cache == nullptr) {
    return false;
  }

  //  Otherwise set capacity and indicate success
  capacity = cap;
  return true;
}

bool PacketCache::resize(size_t cap) {
  //  Don't resize if we will lose elements.
  //  Make users remove elements manually instead.
  if (cap < size) {
    return false;
  }

  //  Assign realloc value to new pointer so we don't lose old pointer on failure
  CacheEntry *new_cache = static_cast<CacheEntry *>(realloc(cache, cap * sizeof(CacheEntry)));

  //  If realloc failed, return false and don't update pointer
  if (new_cache == nullptr) {
    return false;
  }

  //  Everything should be fine; update pointer and capacity and signal success
  cache = new_cache;
  capacity = cap;
  return true;
}

