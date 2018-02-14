# networking
The code that will be running on the arduino that handles stuff such as configuration and incoming packets.

## Packet Structure
Each packet consists of some header data in little endian format followed by the actual data inside the packet. The structure looks like this:

```
Origin ID     : 4 bytes
Destination ID: 4 bytes
Packet Number : 4 bytes
Data length   : 1 byte
Data          : (length) bytes
```

Or more in code,

```cpp
struct Packet {
  uint32_t origin;
  uint32_t destination;
  uint32_t id;
  uint8_t length;
  char *data;
};```

Because of length being a single byte, the maximum size the payload can be is 255 bytes, which means the maximum packet size is 268 when factoring in header data.
