#ifndef PACKET_HPP
#define PACKET_HPP

#include <Arduino.h>
#include <SoftwareSerial.h>

extern SoftwareSerial XBee;

/// Offsets expected for each packet read
enum Offset : uint8_t {
  Origin = 0,
  Destination = 4,
  PacketId = 8,
  Length = 12,
  Payload = 13,
};

/// Holds packet information as well as methods that help
/// to send and receive formatted packets.
struct Packet {
  uint32_t origin;
  uint32_t dest;
  uint32_t id;
  byte *data;
  uint8_t len;
private:
  bool _good;

public:
  /// Default constructor will automatically populate the packet
  /// by calling the read method internally. To determine whether
  /// or not the packet read was well-formatted, call is_good.
  Packet();

  /// Creates a packet for this device given the destination and payload.
  ///
  /// The origin and packet id will be set automatically when created.
  /// The origin is set to the saved internal UUID using Config, and
  /// the packet id is a static uint32_t that increments on creation.
  Packet(uint32_t dest, byte *payload, size_t len);

  /// Used to delete the data that is allocated internally.
  ~Packet();
  
  /// Reads a 4 byte value from the array at a given offset.
  /// Value is read in little endian format.
  ///
  /// There are no bounds checks, so this method may crash if
  /// given an invalid offset.
  ///
  /// Params
  ///   offset: Where the data starts
  /// Returns
  ///   The 4 bytes at offset in data interpretted as a litle endian uint32_t
  uint32_t readU32(size_t offset);

  /// Reads in a packet from the XBee module.
  ///
  /// If the packet read is detected to be malformed, then
  /// the _good flag will be set to false. You can get this flag
  /// from the is_good method.
  bool read();

  /// Sends the current packet to the XBee module.
  ///
  /// This method automatically formats the header using the data
  /// in the class.
  void send();

  /// Returns whether or not the last read call succeeded or not.
  bool is_good();

  /// Returns whether or not this packet is destined for this node.
  bool is_ours();

  /// Prints the packet information to the serial port.
  void debugPrint();
};

#endif
