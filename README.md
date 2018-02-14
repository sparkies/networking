# Networking
The code that will be running on the Arduino that handles stuff such as configuration and incoming packets.

## Packet Structure
Each packet consists of some header data in little endian format followed by the actual data inside the packet. The structure looks like this:

```
Origin ID     : 4 bytes
Destination ID: 4 bytes
Packet Number : 4 bytes
Data length   : 1 byte
Data          : (length) bytes
```

Or in code,

```cpp
struct Packet {
  uint32_t origin;
  uint32_t destination;
  uint32_t id;
  uint8_t length;
  char *data;
};
```

Because of length being a single byte, the maximum size the payload can be is 255 bytes, which means the maximum packet size is 268 when factoring in header data. The data member is not a fixed size in order to help save space in memory.

## Configuration Structure
Configuration settings are stored in EEPROM so they can be stored through power cycles. This information must be updated through an external program, or applied manually by flashing a separate Arduino project which writes the data to memory.

The structure looks like this:

```cpp
struct Settings {
  uint32_t uuid;  //  Unique identifier for this node
  double min_vol; //  Minimum voltage to be read
  double max_vol; //  Maxiumum voltage to be read
  double min_val; //  Minimum value that can be read
  double max_val; //  Maxiumum value that can be read
  char name[100]; //  Name of this node
  char units[10]; //  Unit symbol for the values to be read
}
```

The structure is saved to position 0, so every value's offset is the sum of the sizes of all types that come before it. For instance, `min_vol` would be at offset 4 since `uint32_t` takes up 4 bytes (0 - 3). 

The `name` and `units` members have a fixed length in memory and are expected to be null-terminated strings. Having these be a constant size makes it easier to determine the offset of following data and makes it easier to read and write the structure.  When updated, they will be cleared out to avoid older data from being left in memory.

## Packet Processing
Each packet is first read off of the xbee module into memory. If a packet is malformed, it is simply thrown away with no error-correcting. To avoid infinite rebroadcast loops, the packet id is then taken into consideration. If the packet id for a given origin has already been seen, then it will simply drop the packet. If the packet id for the given origin has not been seen, then the node will check whether its own internal UUID matches the destination. If they match, then the packet is consumed and processed. If the UUID does not match the destination, then the packet is simply re-transmitted for other nodes to pick up.
